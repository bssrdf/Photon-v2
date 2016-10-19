package ui;
import java.awt.BorderLayout;

import javax.swing.JFrame;

public class Window
{
	private JFrame m_jframe;
	private Display m_display;
	
	public Window(int widthPx, int heightPx)
	{
		m_jframe = new JFrame("Photon ver. 0.1");
		m_jframe.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		m_jframe.setBounds(0, 0, widthPx, heightPx);
		m_jframe.setLocationRelativeTo(null);
		m_jframe.setLayout(new BorderLayout());
		m_jframe.setResizable(false);
		m_jframe.setVisible(true);
		
		m_display = new Display(widthPx, heightPx);
		m_jframe.add(m_display, BorderLayout.CENTER);
		
		m_jframe.pack();
		m_jframe.revalidate();
	}
	
//	public void render(HdrFrame frame)
//	{
//		m_display.render(frame);
//	}
}
