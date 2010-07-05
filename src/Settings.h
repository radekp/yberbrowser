/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

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

    void setPrivatePath(QString& path) { m_privatePath = path; }
    QString privatePath() const { return m_privatePath; }

    void setIsFullScreen(bool flag) { m_isFullScreen = flag;}
    bool isFullScreen() const { return m_isFullScreen; }

    QString cookieFilePath() const { return privatePath() + "cookies.dat"; }

private:
    Settings() {
        m_showToolbar = true;
        m_tileVisualizationEnabled = false;
        m_useGL = false;
        m_showFPS = false;
        m_autoCompleteEnabled = true;
        m_tilingEnabled = true;
#if defined(Q_WS_MAEMO_5) || defined(Q_OS_SYMBIAN) || USE_MEEGOTOUCH
        m_isFullScreen = true;
#else
        m_isFullScreen = false;
#endif
    }

    bool m_showToolbar;
    bool m_tileVisualizationEnabled;
    bool m_useGL;
    bool m_showFPS;
    bool m_autoCompleteEnabled;
    bool m_tilingEnabled;
    QString m_privatePath;
    bool m_isFullScreen;
};

#endif

