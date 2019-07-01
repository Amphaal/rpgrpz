#include "YoutubeSignatureDecipherer.h"

QString YoutubeSignatureDecipherer::decipher(const QString &signature) {
    
    auto modifiedSignature = signature;
    auto copyOfOperations = this->_operations;

    while(!copyOfOperations.isEmpty()) {

        auto operation = copyOfOperations.dequeue();
        switch(operation.first) {
            
            case Reverse: {
                std::reverse(modifiedSignature.begin(), modifiedSignature.end());
            }
            break;

            case Slice: {
                auto targetIndex = operation.second.toInt();
                modifiedSignature = modifiedSignature.mid(targetIndex);
            }
            break;

            case Swap: {
                auto firstIndex = 0;
                auto secondIndex = operation.second.toInt();

                auto first = QString(modifiedSignature[firstIndex]);
                auto second = QString(modifiedSignature[secondIndex]);
                
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

YoutubeSignatureDecipherer* YoutubeSignatureDecipherer::create(const QString &clientPlayerUrl, const QString &rawPlayerSourceData) {
    auto newDecipher = new YoutubeSignatureDecipherer(rawPlayerSourceData);
    
    if(_cache.contains(clientPlayerUrl)) {
        _cache[clientPlayerUrl] = newDecipher;
    } else {
        _cache.insert(clientPlayerUrl, newDecipher);
    }
    
    return newDecipher;
}

YoutubeSignatureDecipherer* YoutubeSignatureDecipherer::fromCache(const QString &clientPlayerUrl) {
    return _cache[clientPlayerUrl];
}

YoutubeSignatureDecipherer::YoutubeSignatureDecipherer(const QString &rawPlayerSourceData) {
    
    //find decipher function name
    QRegularExpression findFunctionName("\\bc\\s*&&\\s*d\\.set\\([^,]+,\\s*(?:encodeURIComponent\\s*\\()?\\s*([\\w$]+)\\(");
    auto functionName = findFunctionName.match(rawPlayerSourceData).captured(1);
    
    //get the body of the function
    QRegularExpression findFunctionBody("(?!h\\.)" + QRegularExpression::escape(functionName) + "=function\\(\\w+\\)\\{(.*?)\\}");
    auto functionBody = findFunctionBody.match(rawPlayerSourceData).captured(1);
    
    //calls
    auto javascriptFunctionCalls = functionBody.split(";", QString::SplitBehavior::SkipEmptyParts);

    //prepare
    QHash<CipherOperation, QString> functionNamesByOperation;
    // QRegularExpression findCalledFunction("\\w+(?:.|\\[)(\\\"\"?\\w+(?:\\\"\")?)\\]?\\(");
    QRegularExpression findCalledFunction("\\.(.*?)\\(");
    QRegularExpression findArgument("\\(\\w+,(\\d+)\\)");

    //find subjacent functions used by decipherer
    for(auto &call : javascriptFunctionCalls) {
        
        //once all are found, break
        if(functionNamesByOperation.count() == 3) break;

        //find which function is called
        auto match = findCalledFunction.match(call);
        if(!match.hasMatch()) continue;
        auto calledFunctionName = match.captured(1);

        //custom regexes to find decipherer methods
        auto customRegexes = QHash<CipherOperation, QRegularExpression> {
            { Reverse, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\)")},
            { Slice, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\([a],b\\).(\\breturn\\b)?.?\\w+\\.")},
            { Swap, QRegularExpression(QRegularExpression::escape(calledFunctionName) + ":\\bfunction\\b\\(\\w+\\,\\w\\).\\bvar\\b.\\bc=a\\b")}
        };

        //find...
        QHash<CipherOperation, QRegularExpression>::iterator i;
        for (i = customRegexes.begin(); i != customRegexes.end(); ++i) {

            //if already found, skip
            auto co = i.key();
            if(functionNamesByOperation.contains(co)) continue;

            //check with regex
            auto regex = i.value();
            if(regex.match(rawPlayerSourceData).hasMatch()) {
                functionNamesByOperation.insert(co, calledFunctionName);
            }
    
        }
    }

    //determine order and execution of subjacent methods
    QQueue<QPair<CipherOperation, QVariant>> operations;
    for(auto &call : javascriptFunctionCalls) {

        //find which function is called
        auto match = findCalledFunction.match(call);
        if(!match.hasMatch()) continue;
        auto calledFunctionName = match.captured(1);

        //by operation type
        auto operationType = functionNamesByOperation.key(calledFunctionName);
        switch(operationType) {
            
            case Reverse: {
                operations.enqueue(
                    QPair<CipherOperation, QVariant>(
                        operationType, 
                        QVariant()
                    )
                );
            }
            break;

            case Slice:
            case Swap: {
                auto matchingArg = findArgument.match(call);
                auto arg = matchingArg.captured(1).toInt();
                operations.enqueue(
                    QPair<CipherOperation, QVariant>(
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

    //copy operation to object
    this->_operations = operations;
};