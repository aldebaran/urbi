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

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.sound.sampled.*;

import urbi.UClient;

public class	SendSoundSampler extends JFrame
{
    private	UClient	client;

    public		SendSoundSampler()
    {
	super("Send Sound Sampler");
	setDefaultCloseOperation(EXIT_ON_CLOSE);
	createUI();
	setVisible(true);
	pack();
	setVisible(true);

    }

    public		SendSoundSampler(UClient c)
    {
	super("Send Sound Sampler");
	client = c;
	setDefaultCloseOperation(EXIT_ON_CLOSE);
	createUI();
	setVisible(true);
	pack();
	setVisible(true);

    }

    public void		setClient(UClient c)
    {
	this.client = c;
    }

    public UClient	getClient()
    {
	return client;
    }

    private void	createUI()
    {
	setFont(new Font("Serif", Font.PLAIN, 12));
	setSize(200, 350);
	Container content = getContentPane();

	final JButton play = new JButton("Play");
	play.setEnabled(true);
	ActionListener saveListener = new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
		{
		    FileDialog		fd = new FileDialog(SendSoundSampler.this);
		    fd.setVisible(true);
		    if (fd.getFile() == null)
			return ;
		    String path = fd.getDirectory() + fd.getFile();
		    try
		    {
			SoundUtilities.sendSound(path, client);
		    }
		    catch (IOException ie)
		    {
		    }
		}
	    };
	play.addActionListener(saveListener);
	Panel	bottom = new Panel(new GridLayout(2, 1));
	Panel	topBottom = new Panel();
	topBottom.add(play);
	bottom.add(topBottom);
	content.add(bottom, BorderLayout.CENTER);

	addWindowListener(new WindowAdapter()
	    {
		public void windowClosing(WindowEvent e)
		{
		    dispose();
		    System.exit(0);
		}
	    });
    }
}
