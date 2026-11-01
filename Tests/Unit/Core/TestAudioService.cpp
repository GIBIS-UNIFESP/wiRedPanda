// Copyright 2015 - 2026, GIBIS-UNIFESP and the wiRedPanda contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Tests/Unit/Core/TestAudioService.h"

#include <memory>

#include <QAudioOutput>
#include <QAudioSink>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QMediaPlayer>
#include <QTemporaryDir>

#include "App/Core/AudioService.h"
#include "App/Core/SimulationHost.h"
#include "App/Core/ToneGenerator.h"
#include "App/Element/Element.h"
#include "App/Element/GraphicElements/AudioBox.h"
#include "App/Element/GraphicElements/Buzzer.h"
#include "App/Element/GraphicElements/InputSwitch.h"
#include "App/IO/Serialization.h"
#include "App/IO/SerializationContext.h"
#include "App/Scene/Scene.h"
#include "App/Scene/Workspace.h"
#include "App/Simulation/Simulation.h"
#include "App/Wiring/Connection.h"
#include "App/Wiring/Port.h"
#include "Tests/Common/TestUtils.h"

namespace TestAudioServiceHelpers {

/// A switch wired to an audio element of \a T in \a workspace's scene, the simulation built.
template<typename T>
struct Wired {
    InputSwitch *input = new InputSwitch;
    T *audio = new T;

    explicit Wired(WorkSpace &workspace)
    {
        CircuitBuilder builder(workspace.scene());
        builder.add(input, audio);
        builder.connect(input, 0, audio, 0);
        builder.initSimulation();
    }

    /// Sets the switch to \a on and runs a tick, whose visual push plays or stops the element.
    void set(WorkSpace &workspace, const bool on) const
    {
        input->setOn(on);
        workspace.scene()->simulation()->update();
    }
};

bool sounding(const Element &element)
{
    const auto playback = AudioService::instance().playback(element);
    return playback && playback->sounding;
}

/// Writes \a path, a copy of the built-in sound, so the media backend can read it.
void writeSound(const QString &path)
{
    QFile::remove(path);
    QVERIFY(QFile::copy(QString::fromLatin1(AudioBox::kDefaultAudioPath), path));
    QVERIFY(QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner));
}

} // namespace TestAudioServiceHelpers

using TestAudioServiceHelpers::sounding;
using TestAudioServiceHelpers::Wired;

void TestAudioService::testThePushPlaysAndStopsABuzzer()
{
    // The visual push plays an audio element while its input is Active, and stops it otherwise
    WorkSpace workspace;
    const Wired<Buzzer> wired(workspace);

    wired.set(workspace, true);
    QVERIFY(wired.audio->isPlaying());
    QVERIFY(sounding(*wired.audio));

    wired.set(workspace, false);
    QVERIFY(!wired.audio->isPlaying());
    QVERIFY(!sounding(*wired.audio));

    wired.set(workspace, true);
    QVERIFY(sounding(*wired.audio));
}

void TestAudioService::testThePushTellsTheHostWhatItStartedAndStopped()
{
    // The host hears of each audio element the push starts or stops, once, and of nothing else
    class Host : public SimulationHost
    {
    public:
        QList<QGraphicsItem *> simulationItems() const override { return items; }
        void setMuted(bool) override {}
        void playbackChanged(const QList<Element *> &elements) override { heard.append(elements); }

        QList<QGraphicsItem *> items;
        QList<QList<Element *>> heard;
    };

    InputSwitch input;
    Buzzer buzzer;
    Element led{ElementType::Led};
    auto connection = std::make_unique<Connection>();
    connection->setStartPort(input.outputPort());
    connection->setEndPort(buzzer.inputPort(0));
    auto toLed = std::make_unique<Connection>();
    toLed->setStartPort(input.outputPort());
    toLed->setEndPort(led.inputPort(0));
    Host host;
    host.items = {&input, &buzzer, &led, connection.get(), toLed.get()};
    Simulation simulation(&host);

    simulation.update();
    QVERIFY(host.heard.isEmpty());

    input.setOn(true);
    simulation.update();
    simulation.update();
    QCOMPARE(host.heard.size(), 1);
    QCOMPARE(host.heard.constFirst(), QList<Element *>{&buzzer});
    QVERIFY(buzzer.isPlaying());

    input.setOn(false);
    simulation.update();
    QCOMPARE(host.heard.size(), 2);
    QCOMPARE(host.heard.constLast(), QList<Element *>{&buzzer});
    QVERIFY(!buzzer.isPlaying());
}

void TestAudioService::testALoadLeavesAPlayingVoiceAsItIs()
{
    // A snapshot's restore, as an undo of a property runs, loads the element where it is: it plays on while its input
    // stays Active, and stops on the push that finds it Inactive
    WorkSpace workspace;
    const Wired<Buzzer> wired(workspace);
    wired.set(workspace, true);
    QVERIFY(sounding(*wired.audio));

    QByteArray data;
    {
        QDataStream out(&data, QIODevice::WriteOnly);
        Serialization::writePandaHeader(out);
        wired.audio->save(out, {.purpose = SerializationPurpose::InMemorySnapshot});
    }
    QDataStream in(data);
    const QVersionNumber version = Serialization::readPandaHeader(in);
    QHash<quint64, Port *> portMap;
    auto context = workspace.scene()->deserializationContext(portMap, version, SerializationPurpose::InMemorySnapshot);
    wired.audio->load(in, context);
    workspace.scene()->simulation()->update();
    QVERIFY(wired.audio->isPlaying());
    QVERIFY(sounding(*wired.audio));

    wired.set(workspace, false);
    QVERIFY(!sounding(*wired.audio));
}

void TestAudioService::testAnAudioBoxPlaysItsFileOrTheBuiltInSound()
{
    // An audio box plays its file; one that cannot be read keeps its path while the built-in sound plays in its place,
    // until the box plays with the file back
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("sound.wav");
    TestAudioServiceHelpers::writeSound(path);

    WorkSpace workspace;
    const Wired<AudioBox> wired(workspace);
    wired.audio->setAudio(path);
    wired.set(workspace, true);
    QCOMPARE(AudioService::instance().playback(*wired.audio)->source, path);

    wired.set(workspace, false);
    QVERIFY(QFile::remove(path));
    wired.set(workspace, true);
    QCOMPARE(wired.audio->audio(), path);
    QCOMPARE(AudioService::instance().playback(*wired.audio)->source, QString::fromLatin1(AudioBox::kDefaultAudioPath));

    wired.set(workspace, false);
    TestAudioServiceHelpers::writeSound(path);
    wired.set(workspace, true);
    QCOMPARE(AudioService::instance().playback(*wired.audio)->source, path);
}

void TestAudioService::testMuteIsPerDocument()
{
    // Each document has its own mute, which reaches every audio element of it, a buzzer and an audio box alike
    WorkSpace first;
    WorkSpace second;
    const Wired<Buzzer> buzzer(first);
    const Wired<AudioBox> box(first);
    const Wired<Buzzer> other(second);
    buzzer.set(first, true);
    box.set(first, true);
    other.set(second, true);

    first.scene()->mute(true);
    QVERIFY(AudioService::instance().isMuted(first.scene()));
    QVERIFY(!AudioService::instance().isMuted(second.scene()));
    QVERIFY(AudioService::instance().playback(*buzzer.audio)->muted);
    QVERIFY(AudioService::instance().playback(*box.audio)->muted);
    QVERIFY(!AudioService::instance().playback(*other.audio)->muted);
    QVERIFY(sounding(*buzzer.audio));

    first.scene()->mute(false);
    QVERIFY(!AudioService::instance().playback(*buzzer.audio)->muted);
    QVERIFY(!AudioService::instance().playback(*box.audio)->muted);
}

void TestAudioService::testAnElementAddedToAMutedDocumentStartsMuted()
{
    // An element that starts playing in a muted document, or comes into one playing, is muted from the start
    WorkSpace workspace;
    workspace.scene()->mute(true);
    const Wired<Buzzer> wired(workspace);
    wired.set(workspace, true);
    QVERIFY(AudioService::instance().playback(*wired.audio)->muted);

    auto *added = new Buzzer;
    added->setParam(ParamId::Playing, true);
    workspace.scene()->addItem(added);
    QVERIFY(sounding(*added));
    QVERIFY(AudioService::instance().playback(*added)->muted);
}

void TestAudioService::testAPauseMutesAndAResumeTakesTheUsersMute()
{
    // Pausing the simulation mutes its document; resuming takes the user's mute back
    WorkSpace workspace;
    const Wired<Buzzer> wired(workspace);
    auto *simulation = workspace.scene()->simulation();
    simulation->start();
    wired.set(workspace, true);
    QVERIFY(!AudioService::instance().playback(*wired.audio)->muted);

    simulation->stop();
    QVERIFY(AudioService::instance().playback(*wired.audio)->muted);
    simulation->start();
    QVERIFY(!AudioService::instance().playback(*wired.audio)->muted);

    simulation->setUserMuted(true);
    simulation->stop();
    simulation->start();
    QVERIFY(AudioService::instance().playback(*wired.audio)->muted);
    simulation->stop();
}

void TestAudioService::testAParameterChangedReachesTheVoice()
{
    // A new volume, tone or sound file reaches the voice while it plays
    WorkSpace workspace;
    const Wired<Buzzer> buzzer(workspace);
    buzzer.set(workspace, true);
    buzzer.audio->setVolume(0.2);
    buzzer.audio->setFrequency(880.0);
    QCOMPARE(AudioService::instance().playback(*buzzer.audio)->volume, 0.2);
    QCOMPARE(AudioService::instance().playback(*buzzer.audio)->frequency, 880.0);
    QVERIFY(sounding(*buzzer.audio));

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath("sound.wav");
    TestAudioServiceHelpers::writeSound(path);
    const Wired<AudioBox> box(workspace);
    box.set(workspace, true);
    box.audio->setAudio(path);
    box.audio->setVolume(0.7);
    QCOMPARE(AudioService::instance().playback(*box.audio)->source, path);
    QCOMPARE(AudioService::instance().playback(*box.audio)->volume, 0.7);
}

void TestAudioService::testAnElementRemovedStopsAndOneBackPlays()
{
    // An element that leaves its document plays nothing; one that comes back playing, as a deletion undone does, plays
    WorkSpace workspace;
    const Wired<Buzzer> wired(workspace);
    wired.set(workspace, true);

    workspace.scene()->removeItem(wired.audio);
    QVERIFY(!AudioService::instance().playback(*wired.audio));

    workspace.scene()->addItem(wired.audio);
    QVERIFY(sounding(*wired.audio));
}

void TestAudioService::testADestroyedElementDropsItsVoice()
{
    // An element deleted while it plays takes its voice with it
    WorkSpace workspace;
    auto *buzzer = new Buzzer;
    workspace.scene()->addItem(buzzer);
    buzzer->setParam(ParamId::Playing, true);
    AudioService::instance().sync(*buzzer);
    const Element *gone = buzzer;
    QVERIFY(AudioService::instance().playback(*gone));

    delete buzzer;
    QVERIFY(!AudioService::instance().m_voices.contains(gone));
}

void TestAudioService::testOnlyAnAudioElementThatPlayedGetsAVoice()
{
    // An audio element that never played has no voice, and no other element ever gets one
    WorkSpace workspace;
    const Wired<Buzzer> wired(workspace);
    wired.set(workspace, false);
    QVERIFY(!AudioService::instance().playback(*wired.audio));

    wired.audio->setVolume(0.9);
    QVERIFY(!AudioService::instance().playback(*wired.audio));

    // An element with no ParamId::Playing is not asked for it, which a debug build asserts
    AudioService::instance().sync(*wired.input);
    QVERIFY(!AudioService::instance().playback(*wired.input));
}

void TestAudioService::testTheHardwareFollowsTheVoice()
{
    // With an output device, a buzzer's sink and generator and an audio box's player and output follow the voice
    if (!AudioService::instance().m_hasOutputDevice) {
        QSKIP("No audio output device");
    }

    WorkSpace workspace;
    const Wired<Buzzer> buzzer(workspace);
    buzzer.set(workspace, true);
    buzzer.audio->setFrequency(880.0);
    buzzer.audio->setVolume(0.2);
    const auto &tone = *AudioService::instance().m_voices.at(buzzer.audio);
    QCOMPARE(tone.generator->frequency(), 880.0);
    QVERIFY(tone.generator->isOpen());
    // The sink keeps a float
    QCOMPARE(static_cast<float>(tone.sink->volume()), 0.2F);
    workspace.scene()->mute(true);
    QCOMPARE(tone.sink->volume(), 0.0);
    workspace.scene()->mute(false);
    buzzer.set(workspace, false);
    QVERIFY(!tone.generator->isOpen());

    const Wired<AudioBox> box(workspace);
    box.set(workspace, true);
    box.audio->setVolume(0.3);
    const auto &sound = *AudioService::instance().m_voices.at(box.audio);
    QCOMPARE(sound.output->volume(), 0.3F);
    QCOMPARE(sound.player->source(), QUrl("qrc" + QString::fromLatin1(AudioBox::kDefaultAudioPath)));
    workspace.scene()->mute(true);
    QVERIFY(sound.output->isMuted());
    workspace.scene()->mute(false);
}
