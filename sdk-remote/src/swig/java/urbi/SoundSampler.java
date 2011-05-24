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

/**
 * The SoundSampler class helps to capture, play and record audio data.
 * <p>
 * <p>
 * @author Bastien Saltel
 */

public class	SoundSampler extends JFrame
{
	/** A status value indiucating if the sampler is in capture mode. */
	protected boolean	running;

	private SoundAction		action = null;

	public		SoundSampler()
	{
		super("Sound Sampler");
		setDefaultCloseOperation(EXIT_ON_CLOSE);
		createUI();
		setVisible(true);
		pack();
		setVisible(true);
	}

	public void		setAction(SoundAction action)
	{
		this.action = action;
	}

	/**
	 * Creates the user interface to process the operations.
	 * <p>
	 */
	private void	createUI()
	{
		setFont(new Font("Serif", Font.PLAIN, 12));
		setSize(200, 350);
		Container content = getContentPane();

		final JButton capture = new JButton("Capture");
		final JButton stop = new JButton("Stop");
		final JButton play = new JButton("Play");

		final JButton save = new JButton("Save");

		capture.setEnabled(true);
		stop.setEnabled(false);
		play.setEnabled(false);
		save.setEnabled(false);
		ActionListener captureListener = new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					capture.setEnabled(false);
					stop.setEnabled(true);
					play.setEnabled(false);
					save.setEnabled(false);
					captureAudio();
				}
			};
		capture.addActionListener(captureListener);
		content.add(capture, BorderLayout.NORTH);

		ActionListener stopListener = new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					capture.setEnabled(true);
					stop.setEnabled(false);
					play.setEnabled(true);
					save.setEnabled(true);
					running = false;
					action.stopAudio();
				}
			};
		stop.addActionListener(stopListener);
		content.add(stop, BorderLayout.WEST);

		ActionListener playListener = new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					playAudio();
				}
			};
		play.addActionListener(playListener);
		content.add(play, BorderLayout.EAST);

		ActionListener saveListener = new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					FileDialog		fd = new FileDialog(SoundSampler.this);
					fd.setVisible(true);
					if (fd.getFile() == null)
						return ;
					String path = fd.getDirectory() + fd.getFile();
					saveAudio(path);
				}
			};
		save.addActionListener(saveListener);
		Panel	bottom = new Panel(new GridLayout(2, 1));
		Panel	topBottom = new Panel();
		topBottom.add(save);
		bottom.add(topBottom);
		content.add(bottom, BorderLayout.SOUTH);

		addWindowListener(new WindowAdapter()
			{
				public void windowClosing(WindowEvent e)
				{
					dispose();
					System.exit(0);
				}
			});
	}

	private void captureAudio()
	{
		Runnable runner = new Runnable()
			{
				public void run()
				{
					running = true;
					action.captureAudio();
					while (running)
						{}
				}
			};
		Thread captureThread = new Thread(runner);
		captureThread.start();
	}

	private void playAudio()
	{
		Runnable runner = new Runnable()
			{
				public void run()
				{
					action.playAudio();
				}
			};
		Thread playThread = new Thread(runner);
		playThread.start();
	}

	private void saveAudio(final String path)
	{
		Runnable runner = new Runnable()
			{
				public void run()
				{
					action.saveAudio(path);
				}
			};
		Thread saveThread = new Thread(runner);
		saveThread.start();
	}
}
