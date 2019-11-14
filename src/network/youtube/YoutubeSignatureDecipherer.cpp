#include "YoutubeSignatureDecipherer.h"

QString YoutubeSignatureDecipherer::decipher(const QString &signature) {
    
    auto modifiedSignature = signature;
    auto copyOfOperations = this->_operations;

    while(!copyOfOperations.isEmpty()) {

        switch(auto operationPair = copyOfOperations.dequeue(); operationPair.first) {
            
            case CipherOperation::Reverse: {
                std::reverse(modifiedSignature.begin(), modifiedSignature.end());
            }
            break;

            case CipherOperation::Slice: {
                auto targetIndex = operationPair.second.toInt();
                modifiedSignature = modifiedSignature.mid(targetIndex);
            }
            break;

            case CipherOperation::Swap: {
                auto firstIndex = 0;
                auto secondIndex = operationPair.second.toInt();

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

YoutubeSignatureDecipherer::YTClientMethod YoutubeSignatureDecipherer::_findObfuscatedDecipheringFunctionName(const QString &ytPlayerSourceCode) {
    
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

QHash<YoutubeSignatureDecipherer::CipherOperation, YoutubeSignatureDecipherer::YTClientMethod> YoutubeSignatureDecipherer::_findObfuscatedDecipheringOperationsFunctionName(const QString &ytPlayerSourceCode, QList<QString> &javascriptDecipheringOperations) {
    
    QHash<CipherOperation, YTClientMethod> functionNamesByOperation;
    
    //regex
    auto regex = R"(\w+\.(\w+)\()";
    QRegularExpression findCalledFunction(regex);

    //find subjacent functions used by decipherer
    for(const auto &call : javascriptDecipheringOperations) {
        
        //once all are found, break
        if(functionNamesByOperation.count() == 3) break;

        //find which function is called
        auto match = findCalledFunction.match(call);
        if(!match.hasMatch()) continue;
        auto calledFunctionName = match.captured(1);

        //custom regexes to find decipherer methods
        QHash<YoutubeSignatureDecipherer::CipherOperation, QRegularExpression> customRegexes {
            { CipherOperation::Reverse, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\)")},
            { CipherOperation::Slice, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\([a],b\\).(\\breturn\\b)?.?\\w+\\.")},
            { CipherOperation::Swap, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\,\\w\\).\\bvar\\b.\\bc=a\\b")}
        };

        //find...
        for (auto i = customRegexes.begin(); i != customRegexes.end(); ++i) {

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

YoutubeSignatureDecipherer::YTDecipheringOperations YoutubeSignatureDecipherer::_buildOperations(
        QHash<CipherOperation, YTClientMethod> &functionNamesByOperation,
        QList<QString> &javascriptOperations
    ) {
    
    YTDecipheringOperations operations;

    //determine order and execution of subjacent methods
    auto regex = R"(\.(\w+)\(\w+,(\d+)\))";
    QRegularExpression findFuncAndArgument(regex);
    
    //iterate
    for(const auto &call : javascriptOperations) {

        //find which function is called
        auto match = findFuncAndArgument.match(call);
        if(!match.hasMatch()) continue;

        auto calledFunctionName = match.captured(1);
        auto arg = match.captured(2).toInt();

        //by operation type
        switch(auto operationType = functionNamesByOperation.key(calledFunctionName)) {
            
            case CipherOperation::Reverse: {
                operations.enqueue({operationType, QVariant()});
            }
            break;

            case CipherOperation::Slice:
            case CipherOperation::Swap: {
                operations.enqueue({operationType, QVariant(arg)});
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
    auto functionName = _findObfuscatedDecipheringFunctionName(ytPlayerSourceCode);

    //get JS deciphering operations
    auto javascriptOperations = _findJSDecipheringOperations(ytPlayerSourceCode, functionName);

    //find operations functions name
    auto functionNamesByOperation = _findObfuscatedDecipheringOperationsFunctionName(ytPlayerSourceCode, javascriptOperations);

    //generate operations
    auto operations = _buildOperations(functionNamesByOperation, javascriptOperations);

    //copy operation to object
    this->_operations = operations;

};

inline uint qHash(const YoutubeSignatureDecipherer::CipherOperation &key, uint seed = 0) {return uint(key) ^ seed;}
