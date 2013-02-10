#ifndef GAME_NOTIFICATION_LOG
#define GAME_NOTIFICATION_LOG

#include "Protocol.h"
#include "Texture.h"

#include <bass.h>
#include <vector>

struct Font;
class Particles;
class Map;

class NotificationLog
{
public:

    NotificationLog(Map* map, Particles* mapParticles, Font* font, int xSize, int ySize);
    ~NotificationLog();

    void Draw();
    void OnMouseDown(int x, int y, int button);
    void OnMouseUp(int x, int y, int button);
    void OnMouseMove(int x, int y);
    void LoadResources();

    void AddNotification(float time, const Protocol::NotificationPacket& packet);

private:

    void PlaySample(HSAMPLE sample);

    void VisualizeNotification(const Protocol::NotificationPacket& packet);   
    void AddNotificationParticle(Texture* texture, int x, int y);
    int GetEntryUnderCursor(int x, int y);

    struct LogEntry
    {
        float time;
        Protocol::NotificationPacket packet;
    };

    Font*                   m_font;

    int                     m_windowX;
    int                     m_windowY;

    Texture                 m_notificationAgentLost;
    Texture                 m_notificationAgentCaptured;
    Texture                 m_notificationAgentSpotted;
    Texture                 m_notificationCrime;
    Texture                 m_notificationBuildingDestroyed;
    Texture                 m_notificationIntelDetected;

    HSAMPLE                 m_soundCrime;
    HSAMPLE                 m_soundSpotted;
    HSAMPLE                 m_soundDestroyed;

    Map*                    m_map;
    Particles*              m_mapParticles;
    std::vector<LogEntry>   m_entries;
    int                     m_activeEntry;
    int                     m_rowHeight;
    int                     m_firstEntry;
};

#endif
