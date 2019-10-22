#include "YoutubeSignatureDecipherer.h"

QString YoutubeSignatureDecipherer::decipher(const QString &signature) {
    
    auto modifiedSignature = signature;
    auto copyOfOperations = this->_operations;

    while(!copyOfOperations.isEmpty()) {

        auto operation = copyOfOperations.dequeue();
        switch(operation.first) {
            
            case YoutubeSignatureDecipherer::CipherOperation::Reverse: {
                std::reverse(modifiedSignature.begin(), modifiedSignature.end());
            }
            break;

            case YoutubeSignatureDecipherer::CipherOperation::Slice: {
                auto targetIndex = operation.second.toInt();
                modifiedSignature = modifiedSignature.mid(targetIndex);
            }
            break;

            case YoutubeSignatureDecipherer::CipherOperation::Swap: {
                auto firstIndex = 0;
                auto secondIndex = operation.second.toInt();

                auto first = QString(modifiedSignature.at(firstIndex));
                auto second = QString(modifiedSignature.at(secondIndex));
                
                modifiedSignature.replace(firstIndex, 1, second);
                modifiedSignature.replace(secondIndex, 1, first);
            }
            break;

            default:
                break;

        }

    }

    return modifiedSignature;
};

YoutubeSignatureDecipherer* YoutubeSignatureDecipherer::create(const QString &clientPlayerUrl, const QString &ytPlayerSourceCode) {
    auto newDecipher = new YoutubeSignatureDecipherer(ytPlayerSourceCode);
    
     _cache.insert(clientPlayerUrl, newDecipher);
    
    return newDecipher;
}

YoutubeSignatureDecipherer* YoutubeSignatureDecipherer::fromCache(const QString &clientPlayerUrl) {
    return _cache.value(clientPlayerUrl);
}

YTClientMethod YoutubeSignatureDecipherer::_findObfuscatedDecipheringFunctionName(const QString &ytPlayerSourceCode) {
    
    auto regex = R"((\w+)=function\(\w+\){(\w+)=\2\.split\(\x22{2}\);.*?return\s+\2\.join\(\x22{2}\)})";
    QRegularExpression findFunctionName(regex);
    
    auto match = findFunctionName.match(ytPlayerSourceCode);
    auto functionName = match.captured(1);
    
    if(functionName.isEmpty()) {
        throw std::runtime_error("Youtube : [Decipherer] No function name found !");
    }
    
    // qDebug() << "Youtube : decipherer function name >> " << functionName;

    return functionName;
}

QList<QString> YoutubeSignatureDecipherer::_findJSDecipheringOperations(const QString &ytPlayerSourceCode, const YTClientMethod &obfuscatedDecipheringFunctionName) {

    //get the body of the function
    auto regex = QString(R"((?!h\.)%1=function\(\w+\)\{(.*?)\})");
    auto escapedArg = QRegularExpression::escape(obfuscatedDecipheringFunctionName);
    regex = regex.arg(escapedArg);

    QRegularExpression findFunctionBody(regex);
    auto match = findFunctionBody.match(ytPlayerSourceCode);

    auto functionBody = match.captured(1);
    if(functionBody.isEmpty()) {
        throw std::runtime_error("Youtube : [Decipherer] No function body found !");
    }

    //calls
    auto javascriptFunctionCalls = functionBody.split(";", QString::SplitBehavior::SkipEmptyParts);

    // qDebug() << "Youtube : decipherer function body parts >>" << functionBody;

    return javascriptFunctionCalls;
}

QHash<YoutubeSignatureDecipherer::CipherOperation, YTClientMethod> YoutubeSignatureDecipherer::_findObfuscatedDecipheringOperationsFunctionName(const QString &ytPlayerSourceCode, QList<QString> &javascriptDecipheringOperations) {
    
    QHash<YoutubeSignatureDecipherer::CipherOperation, YTClientMethod> functionNamesByOperation;
    
    //regex
    auto regex = R"(\w+\.(\w+)\()";
    QRegularExpression findCalledFunction(regex);

    //find subjacent functions used by decipherer
    for(auto &call : javascriptDecipheringOperations) {
        
        //once all are found, break
        if(functionNamesByOperation.count() == 3) break;

        //find which function is called
        auto match = findCalledFunction.match(call);
        if(!match.hasMatch()) continue;
        auto calledFunctionName = match.captured(1);

        //custom regexes to find decipherer methods
        auto customRegexes = QHash<YoutubeSignatureDecipherer::CipherOperation, QRegularExpression> {
            { YoutubeSignatureDecipherer::CipherOperation::Reverse, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\)")},
            { YoutubeSignatureDecipherer::CipherOperation::Slice, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\([a],b\\).(\\breturn\\b)?.?\\w+\\.")},
            { YoutubeSignatureDecipherer::CipherOperation::Swap, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\,\\w\\).\\bvar\\b.\\bc=a\\b")}
        };

        //find...
        QHash<YoutubeSignatureDecipherer::CipherOperation, QRegularExpression>::iterator i;
        for (i = customRegexes.begin(); i != customRegexes.end(); ++i) {

            //if already found, skip
            auto co = i.key();
            if(functionNamesByOperation.contains(co)) continue;

            //check with regex
            auto regex = i.value();
            if(regex.match(ytPlayerSourceCode).hasMatch()) {
                functionNamesByOperation.insert(co, calledFunctionName);
            }
    
        }
    }

    if(!functionNamesByOperation.count()) {
        throw std::runtime_error("Youtube : [Decipherer] Missing function names by operations !");
    }

    // qDebug() << "Youtube :" << functionNamesByOperation;
    
    return functionNamesByOperation;
}

YTDecipheringOperations YoutubeSignatureDecipherer::_buildOperations(
        QHash<YoutubeSignatureDecipherer::CipherOperation, YTClientMethod> &functionNamesByOperation,
        QList<QString> &javascriptOperations
    ) {
    
    YTDecipheringOperations operations;

    //determine order and execution of subjacent methods
    auto regex = R"(\.(\w+)\(\w+,(\d+)\))";
    QRegularExpression findFuncAndArgument(regex);
    
    //iterate
    for(auto &call : javascriptOperations) {

        //find which function is called
        auto match = findFuncAndArgument.match(call);
        if(!match.hasMatch()) continue;

        auto calledFunctionName = match.captured(1);
        auto arg = match.captured(2).toInt();

        //by operation type
        auto operationType = functionNamesByOperation.key(calledFunctionName);
        switch(operationType) {
            
            case YoutubeSignatureDecipherer::CipherOperation::Reverse: {
                operations.enqueue(
                    QPair<YoutubeSignatureDecipherer::CipherOperation, QVariant>(
                        operationType, 
                        QVariant()
                    )
                );
            }
            break;

            case YoutubeSignatureDecipherer::CipherOperation::Slice:
            case YoutubeSignatureDecipherer::CipherOperation::Swap: {
                operations.enqueue(
                    QPair<YoutubeSignatureDecipherer::CipherOperation, QVariant>(
                        operationType, 
                        QVariant(arg)
                    )
                );
            }
            break;

            default:
                break;

        }
    }

    if(!operations.count()) {
        throw std::runtime_error("Youtube : [Decipherer] No operations found !");
    }
    
    // qDebug() << "Youtube >>" << operations;
    
    return operations;
}

YoutubeSignatureDecipherer::YoutubeSignatureDecipherer(const QString &ytPlayerSourceCode) {
    
    //find deciphering function name
    auto functionName = YoutubeSignatureDecipherer::_findObfuscatedDecipheringFunctionName(ytPlayerSourceCode);

    //get JS deciphering operations
    auto javascriptOperations = YoutubeSignatureDecipherer::_findJSDecipheringOperations(ytPlayerSourceCode, functionName);

    //find operations functions name
    auto functionNamesByOperation = YoutubeSignatureDecipherer::_findObfuscatedDecipheringOperationsFunctionName(ytPlayerSourceCode, javascriptOperations);

    //generate operations
    auto operations = YoutubeSignatureDecipherer::_buildOperations(functionNamesByOperation, javascriptOperations);

    //copy operation to object
    this->_operations = operations;

};
