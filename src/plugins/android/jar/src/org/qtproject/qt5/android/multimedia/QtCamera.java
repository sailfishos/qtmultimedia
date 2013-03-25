/****************************************************************************
 **
 ** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the QtMultimedia module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.  For licensing terms and
 ** conditions see http://qt.digia.com/licensing.  For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights.  These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

package org.qtproject.qt5.android.multimedia;

import android.hardware.Camera;
import android.graphics.SurfaceTexture;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.FragmentManager;
import android.util.Log;

public class QtCamera extends Fragment
{
    static private FragmentManager m_fragmentManager = null;
    private int m_cameraId = -1;
    private Camera m_camera = null;

    private static final String TAG = "Qt Camera";

    static public void setActivity(Activity activity, Object activityDelegate)
    {
        m_fragmentManager = activity.getFragmentManager();
    }

    public QtCamera()
    {
    }

    private QtCamera(int id, Camera cam)
    {
        m_cameraId = id;
        m_camera = cam;

        FragmentTransaction fragmentTransaction = m_fragmentManager.beginTransaction();
        fragmentTransaction.add(this, "QtCameraFragment");
        fragmentTransaction.commit();
    }

    public static QtCamera open(int cameraId)
    {
        try {
            Camera cam = Camera.open(cameraId);
            return new QtCamera(cameraId, cam);
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
        return null;
    }

    public Camera.Parameters getParameters()
    {
        return m_camera.getParameters();
    }

    public void lock()
    {
        try {
            m_camera.lock();
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
    }

    public void unlock()
    {
        try {
            m_camera.unlock();
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
    }

    public void release()
    {
        m_camera.release();
    }

    public void destroy()
    {
        FragmentTransaction fragmentTransaction = m_fragmentManager.beginTransaction();
        fragmentTransaction.remove(this);
        fragmentTransaction.commit();
    }

    public void reconnect()
    {
        try {
            m_camera.reconnect();
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
    }

    public void setDisplayOrientation(int degrees)
    {
        m_camera.setDisplayOrientation(degrees);
    }

    public void setParameters(Camera.Parameters params)
    {
        try {
            m_camera.setParameters(params);
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
    }

    public void setPreviewTexture(SurfaceTexture surfaceTexture)
    {
        try {
            m_camera.setPreviewTexture(surfaceTexture);
        } catch(Exception e) {
            Log.d(TAG, e.getMessage());
        }
    }

    public void startPreview()
    {
        m_camera.startPreview();
    }

    public void stopPreview()
    {
        m_camera.stopPreview();
    }

    @Override
    public void onPause()
    {
        notifyPause(m_cameraId);
        super.onPause();
    }

    @Override
    public void onResume()
    {
        notifyResume(m_cameraId);
        super.onResume();
    }

    private static native void notifyPause(int id);
    private static native void notifyResume(int id);
}
