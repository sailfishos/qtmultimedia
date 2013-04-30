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

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.FragmentManager;

public class QtActivityStateListener extends Fragment
{
    static private FragmentManager m_fragmentManager = null;
    private long m_Id = -1;

    static public void setActivity(Activity activity, Object activityDelegate)
    {
        m_fragmentManager = activity.getFragmentManager();
    }

    public QtActivityStateListener()
    {
        super();
    }

    public QtActivityStateListener(long id)
    {
        super();
        m_Id = id;
    }

    public void create()
    {
        FragmentTransaction fragmentTransaction = m_fragmentManager.beginTransaction();
        fragmentTransaction.add(this, "QtActivityStateListener");
        fragmentTransaction.commit();
    }

    public void destroy()
    {
        FragmentTransaction fragmentTransaction = m_fragmentManager.beginTransaction();
        fragmentTransaction.remove(this);
        fragmentTransaction.commit();
    }

    @Override
    public void onPause()
    {
        notifyPause(m_Id);
        super.onPause();
    }

    @Override
    public void onResume()
    {
        notifyResume(m_Id);
        super.onResume();
    }

    private static native void notifyPause(long id);
    private static native void notifyResume(long id);
}
