#pragma once

#include <QFrame>

namespace Phonon
{
    class VolumeSlider;
    class AudioOutput;
}

class VolumeSlider : public QFrame
{
    Q_OBJECT

public:
    explicit VolumeSlider( Phonon::AudioOutput* audioOutput, QWidget* parent = 0 );
    void setAudioOutput( Phonon::AudioOutput* audioOutput );

public slots:
    void animateOpen();
    void animateClose();

private:
    void paintEvent( QPaintEvent* event );

private slots:
    void onFrameChanged( int frame );
    void onFinished();

private:
    struct
    {
        Phonon::VolumeSlider* volumeSlider;
    } ui;

    class QTimeLine* m_timeLine;
};

