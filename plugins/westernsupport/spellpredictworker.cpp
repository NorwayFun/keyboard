/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "spellpredictworker.h"

#include <QDebug>

SpellPredictWorker::SpellPredictWorker(QObject *parent)
    : QObject(parent)
    , m_candidatesContext()
    , m_presageCandidates(CandidatesCallback(m_candidatesContext))
    , m_presage(&m_presageCandidates)
    , m_spellChecker()
    , m_word()
    , m_limit(5)
    , m_processingWords(false)
{
    m_presage.config("Presage.Selector.SUGGESTIONS", "6");
    m_presage.config("Presage.Selector.REPEAT_SUGGESTIONS", "yes");
}

void SpellPredictWorker::parsePredictionText(const QString& surroundingLeft, const QString& preedit)
{
    m_candidatesContext = (surroundingLeft.toStdString() + preedit.toStdString());

    QStringList list;

    // If the user input is spelt correctly add it to the start of the predictions
    if(m_spellChecker.spell(preedit)) {
        list << preedit;
    }

    try {
        const std::vector<std::string> predictions = m_presage.predict();

        std::vector<std::string>::const_iterator it;
        for (it = predictions.begin(); it != predictions.end(); ++it) {
            QString prediction = QString::fromStdString(*it);
            // Presage will implicitly learn any words the user types as part
            // of its prediction model, so we only provide predictions for 
            // words that have been explicitly added to the spellcheck dictionary.
            QString predictionTitleCase = prediction;
            predictionTitleCase[0] = prediction.at(0).toUpper();
            if (m_spellChecker.spell(prediction) || m_spellChecker.spell(predictionTitleCase) || m_spellChecker.spell(prediction.toUpper())) {
                list << prediction;
            }
        }

    } catch (int error) {
        qWarning() << "An exception was thrown in libpresage when calling predict(), exception nr: " << error;
    }

    Q_EMIT newPredictionSuggestions(preedit, list);
}

void SpellPredictWorker::setLanguage(QString locale)
{
    QString dbFileName = "database_"+locale+".db";
    QString fullPath("/usr/share/maliit/plugins/com/ubuntu/lib/"+locale+"/");
    fullPath.append(dbFileName);
    m_spellChecker.setLanguage(locale);
    m_spellChecker.setEnabled(true);

    try {
        m_presage.config("Presage.Predictors.DefaultSmoothedNgramPredictor.DBFILENAME", fullPath.toLatin1().data());
    } catch (int error) {
        qWarning() << "An exception was thrown in libpresage when changing language database, exception nr: " << error;
    }
}

void SpellPredictWorker::suggest(const QString& word, int limit)
{
    if(!m_spellChecker.spell(word)) {
        QStringList suggestions = m_spellChecker.suggest(word, limit);
        Q_EMIT newSpellingSuggestions(word, suggestions);
    }
}

void SpellPredictWorker::newSpellCheckWord(QString word)
{
    // Run through all the words queued in the event loop
    // so we only fetch suggestions for the latest word
    bool setProcessingWords = false;
    if(m_processingWords == false) {
        setProcessingWords = true;
        m_processingWords = true;
    }
    QCoreApplication::processEvents();
    if(setProcessingWords == true) {
        m_processingWords = false;
    }

    m_word = word;

    if(!m_processingWords) {
        suggest(m_word, m_limit);
    }
}

void SpellPredictWorker::addToUserWordList(const QString& word)
{
    m_spellChecker.addToUserWordList(word);
}

void SpellPredictWorker::setSpellCheckLimit(int limit)
{
    m_limit = limit;
}
