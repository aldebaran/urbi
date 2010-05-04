/*! \file SoundAction.java
 *******************************************************************************

 File: SoundAction.java
 Implementation of the SoundAction class.

 This file is part of 
 liburbi
 (c) Bastien Saltel, 2004.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://urbi.sourceforge.net

 **************************************************************************** */

package liburbi.sound;

public interface		SoundAction
{
	public void		captureAudio();

	public void		playAudio();

	public void		stopAudio();

	public void		saveAudio(String path);
}
