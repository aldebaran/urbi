/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
package tests.timer;

import urbi.Log;
import urbi.UObject;
import urbi.UVar;
import urbi.TimerHandle;

public class Timer extends UObject
{
    static {
	UStartRename(Timer.class, "timer");
    }

    public Timer(String name)
    {
	super(name);
	UBindVar(updated, "updated");
	UBindVar(timerup, "timerup");
	UBindVar(hupdated, "hupdated");
	hupdated.setValue(0);
	timerup.setValue(0);
	updated.setValue(0);
	UBindFunction("setupUpdate");
	UBindFunction("setupTimer");
	UBindFunction("unsetupTimer");
	UBindFunction("init");
    }
    public int init()
    {
	return 0;
    }
    public int setupUpdate(int d)
    {
	USetUpdate(d);
	return 0;
    }
    public String setupTimer(int d)
    {
	return USetTimer(d, this, "onTimer");
    }
    public boolean unsetupTimer(String s)
    {
	return removeTimer(TimerHandle.create(s));
    }
    public int update()
    {
	updated.setValue(updated.intValue() + 1);
	return 0;
    }
    public int onTimer()
    {
	timerup.setValue(timerup.intValue() + 1);
	return 0;
    }
    UVar updated = new UVar();
    UVar hupdated = new UVar();
    UVar timerup = new UVar();
};
