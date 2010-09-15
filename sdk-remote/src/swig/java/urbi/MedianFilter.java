package urbi;

import java.util.Vector;

public class	MedianFilter
{
	private	Vector	tab;

	private double		size = 0;

	public MedianFilter(int size)
	{
		this.size = size;
		tab = new Vector();
	}

	public boolean	checkSize()
	{
		if (tab.size() == size)
			return true;
		return false;
	}

	public double		getMedian()
	{
		Double median = (Double)tab.get((int)(size / 2));
		tab.removeAllElements();
		return median.doubleValue();
	}

	public void		addElement(double elt)
	{
		if (tab.size() == 0)
			tab.add(new Double(elt));
		else
			{
				int		i;

				for (i = 0; i < tab.size() && elt > ((Double)tab.get(i)).doubleValue(); i++)
					;

				tab.add(i, new Double(elt));
			}
	}

	public String toString()
	{
		return tab.toString();
	}
}
