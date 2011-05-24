/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

package urbi;

public interface		SoundAction
{
        public void		captureAudio();

        public void		playAudio();

        public void		stopAudio();

        public void		saveAudio(String path);
}
