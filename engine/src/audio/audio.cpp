/*
  Q Light Controller
  audio.cpp

  Copyright (c) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QDomDocument>
#include <QDomElement>
#include <QDebug>
#include <QFile>

#ifdef QT_PHONON_LIB
#include <phonon/mediaobject.h>
#include <phonon/backendcapabilities.h>
#endif

#include "audiodecoder.h"
#ifdef HAS_LIBSNDFILE
  #include "audiodecoder_sndfile.h"
#endif
#ifdef HAS_LIBMAD
  #include "audiodecoder_mad.h"
#endif

#include "audiorenderer.h"
#include "audiorenderer_alsa.h"
#include "audio.h"
#include "doc.h"

#define KXMLQLCAudioSource "Source"
#define KXMLQLCAudioStartTime "StartTime"
#define KXMLQLCAudioColor "Color"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Audio::Audio(Doc* doc)
  : Function(doc, Function::Audio)
#ifdef QT_PHONON_LIB
  , m_object(NULL)
#endif
  , m_startTime(UINT_MAX)
  , m_color(96, 128, 83)
  , m_sourceFileName("")
  , m_audioDuration(0)
{
    setName(tr("New Audio"));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
}

Audio::~Audio()
{

}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Audio::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Audio(doc);
    if (copy->copyFrom(this) == false || doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Audio::copyFrom(const Function* function)
{
    const Audio* aud = qobject_cast<const Audio*> (function);
    if (aud == NULL)
        return false;

    return Function::copyFrom(function);
}

QStringList Audio::getCapabilities()
{
    QStringList cap;
#ifdef QT_PHONON_LIB
    return Phonon::BackendCapabilities::availableMimeTypes();
#endif
#ifdef HAS_LIBSNDFILE
    cap << AudioDecoderSndFile::getSupportedFormats();
#endif
#ifdef HAS_LIBMAD
    cap << AudioDecoderMAD::getSupportedFormats();
#endif
    return cap;
}

/*********************************************************************
 * Properties
 *********************************************************************/
void Audio::setStartTime(quint32 time)
{
    m_startTime = time;
}

quint32 Audio::getStartTime() const
{
    return m_startTime;
}

qint64 Audio::getDuration()
{
    return m_audioDuration;
}

void Audio::setColor(QColor color)
{
    m_color = color;
}

QColor Audio::getColor()
{
    return m_color;
}

bool Audio::setSourceFileName(QString filename)
{
    if (m_sourceFileName.isEmpty() == false)
    {
        // unload previous source
    }
#ifdef QT_PHONON_LIB
    m_object = Phonon::createPlayer(Phonon::MusicCategory,
                                    Phonon::MediaSource(filename));
    if (m_object == NULL)
        return false;
#endif
    m_sourceFileName = filename;
    QFileInfo ai(m_sourceFileName);
    setName(ai.completeBaseName());
#ifdef QT_PHONON_LIB
    connect(m_object, SIGNAL(totalTimeChanged(qint64)), this, SLOT(slotTotalTimeChanged(qint64)));
#endif

#ifdef HAS_LIBSNDFILE
    m_decoder = new AudioDecoderSndFile(m_sourceFileName);
    if (m_decoder->initialize() == false)
        delete m_decoder;
    else
    {
        m_audioDuration = m_decoder->totalTime();
        return true;
    }
#endif
#ifdef HAS_LIBMAD
    m_decoder = new AudioDecoderMAD(m_sourceFileName);
    if (m_decoder->initialize() == false)
        delete m_decoder;
    else
    {
        m_audioDuration = m_decoder->totalTime();
        return true;
    }
#endif
    return false;
}

void Audio::slotTotalTimeChanged(qint64)
{
#ifdef QT_PHONON_LIB
    m_audioDuration = m_object->totalTime();
#endif
    qDebug() << "Audio duration: " << m_audioDuration;
    emit totalTimeChanged(m_audioDuration);
}

void Audio::slotFunctionRemoved(quint32 fid)
{
    Q_UNUSED(fid)
}

bool Audio::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    QDomElement source = doc->createElement(KXMLQLCAudioSource);
    source.setAttribute(KXMLQLCAudioStartTime, m_startTime);
    source.setAttribute(KXMLQLCAudioColor, m_color.name());
    QDomText text = doc->createTextNode(m_sourceFileName);
    source.appendChild(text);
    root.appendChild(source);

    return true;
}

bool Audio::loadXML(const QDomElement& root)
{
    if (root.tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attribute(KXMLQLCFunctionType) != typeToString(Function::Audio))
    {
        qWarning() << Q_FUNC_INFO << root.attribute(KXMLQLCFunctionType)
                   << "is not Audio";
        return false;
    }

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();
        if (tag.tagName() == KXMLQLCAudioSource)
        {
            if (tag.hasAttribute(KXMLQLCAudioStartTime))
                m_startTime = tag.attribute(KXMLQLCAudioStartTime).toUInt();
            if (tag.hasAttribute(KXMLQLCAudioColor))
                m_color = QColor(tag.attribute(KXMLQLCAudioColor));
            m_sourceFileName = tag.text();
        }
    }

    return true;
}

void Audio::postLoad()
{
}

/*********************************************************************
 * Running
 *********************************************************************/
void Audio::preRun(MasterTimer* timer)
{
#ifdef QT_PHONON_LIB
    if (m_object != NULL)
    {
        m_object->play();
        return;
    }
#endif
    if (m_decoder != NULL)
    {
        AudioParameters ap = m_decoder->audioParameters();
        m_audio_out = new AudioRendererAlsa();
        m_audio_out->setDecoder(m_decoder);
        m_audio_out->initialize(ap.sampleRate(), ap.channels(), ap.format());
        /*
        for (int i = 0; i < 5000; i++)
        {
            unsigned char data[128];
            qint64 bytes_read = m_decoder->read((char *)data, 128);
            qint64 bytes_written = m_audio_out->writeAudio(data, bytes_read);
            qDebug() << "[Cycle] read: " << bytes_read << ", written: " << bytes_written;
        }
        */
        m_audio_out->start();
    }
    Function::preRun(timer);
}

void Audio::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)
}

void Audio::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer)
    Q_UNUSED(universes)
#ifdef QT_PHONON_LIB
    if (m_object != NULL)
        m_object->stop();
#endif
    if (m_audio_out != NULL)
        m_audio_out->stop();
}
