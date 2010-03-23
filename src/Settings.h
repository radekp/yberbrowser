#ifndef Settings_h_
#define Settings_h_

#include <QString>

class Settings {
public:
    static Settings* instance() {
        static Settings* instance = 0;
        if (!instance)
            instance = new Settings();
        return instance;
    }

    void enableToolbar(bool enable) { m_showToolbar = enable; }
    bool toolbarEnabled() const { return m_showToolbar; }

    void enableTileVisualization(bool enable) { m_tileVisualizationEnabled = enable; }
    bool tileVisualizationEnabled() const { return m_tileVisualizationEnabled; }

    void setUseGL(bool use) { m_useGL = use; }
    bool useGL() const { return m_useGL; }

    void enableFPS(bool show) { m_showFPS = show; }
    bool FPSEnabled() const { return m_showFPS; }

    void enableAutoComplete(bool enable) { m_autoCompleteEnabled = enable; }
    bool autoCompleteEnabled() const { return m_autoCompleteEnabled; }

    void enableTileCache(bool enable) { m_tilingEnabled = enable; }
    bool tileCacheEnabled() const { return m_tilingEnabled; }

    void enableEngineThread(bool enable) { m_engineThreadEnabled = enable; }
    bool engineThreadEnabled() const { return m_engineThreadEnabled; }
    
    void setPrivatePath(QString& path) { m_privatePath = path; }
    QString privatePath() const { return m_privatePath; }

    void setIsFullScreen(bool flag) { m_isFullScreen = flag;}
    bool isFullScreen() const { return m_isFullScreen; }

private:
    Settings() {
        m_showToolbar = true;
        m_tileVisualizationEnabled = false;
        m_useGL = false;
        m_showFPS = false;
        m_autoCompleteEnabled = true;
        m_tilingEnabled = true;
        m_engineThreadEnabled = false;
        m_isFullScreen = true;
    }

    bool m_showToolbar;
    bool m_tileVisualizationEnabled;
    bool m_useGL;
    bool m_showFPS;
    bool m_autoCompleteEnabled;
    bool m_tilingEnabled;
    bool m_engineThreadEnabled;
    QString m_privatePath;
    bool m_isFullScreen;
};

#endif

