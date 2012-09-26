#ifndef TRACKWIDGET_H
#define TRACKWIDGET_H

#include <QPushButton>

#include <lastfm/Track.h>

namespace Ui { class TrackWidget; }

class QMovie;
class TrackImageFetcher;

class TrackWidget : public QPushButton
{
    Q_OBJECT
    
public:
    explicit TrackWidget( Track& track, QWidget *parent = 0 );
    ~TrackWidget();

    void setTrack( lastfm::Track& track );
    lastfm::Track track() const;

    void setNowPlaying( bool nowPlaying );

public slots:
    void startSpinner();
    void clearSpinner();

signals:
    void clicked( TrackWidget& trackWidget );
    void removed();

private slots:
    void onClicked();
    void onLoveClicked( bool loved );
    void onTagClicked();
    void onShareClicked();

    void onBuyClicked();
    void onGotBuyLinks();

    void onRemoveClicked();
    void onRemovedScrobble();

    void onBuyActionTriggered( QAction* buyAction );

    void onShareLastFm();
    void onShareTwitter();
    void onShareFacebook();

    // signals from the track
    void onLoveToggled( bool loved );
    void onScrobbleStatusChanged();
    void onCorrected( QString correction );

    void play();
    void playNext();

    void updateTimestamp();

public:
    QSize sizeHint() const;

private:
    QString price( const QString& price, const QString& currency ) const;

    void setTrackDetails();
    void setTrackTitleWidth();

    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void contextMenuEvent( QContextMenuEvent* event );

    void fetchAlbumArt();

private:
    Ui::TrackWidget* ui;

    lastfm::Track m_track;

    QPointer<QMovie> m_movie;
    QPointer<QTimer> m_timestampTimer;
    QPointer<TrackImageFetcher> m_trackImageFetcher;

    bool m_nowPlaying;
    bool m_triedFetchAlbumArt;

    QPointer<QMovie> m_spinnerMovie;
    class QLabel* m_spinner;
};

#endif // TRACKWIDGET_H
