using System;
using System.Runtime.InteropServices;
using System.IO;
using System.Diagnostics;
using UnityEngine;
using System.Threading;

public class Transparency : MonoBehaviour
{
	public int width = 512;
	public int height = 512;
	[SerializeField]
	private Material m_Material;

	private RenderTexture blit_Texture;


	// Define function signatures to import from Windows APIs
#if UNITY_STANDALONE_WIN
	private struct MARGINS
	{
		public int cxLeftWidth;
		public int cxRightWidth;
		public int cyTopHeight;
		public int cyBottomHeight;
	}

	[DllImport("user32.dll")]
	private static extern IntPtr GetActiveWindow();

	[DllImport("user32.dll")]
	private static extern int SetWindowLong(IntPtr hWnd, int nIndex, uint dwNewLong);

	[DllImport("Dwmapi.dll")]
	private static extern uint DwmExtendFrameIntoClientArea(IntPtr hWnd, ref MARGINS margins);

	// Definitions of window styles
	const int GWL_STYLE = -16;
	const uint WS_POPUP = 0x80000000;
	const uint WS_VISIBLE = 0x10000000;

	const int BACKGROUND_COLOR = 0;
#endif

#if UNITY_STANDALONE_LINUX
	[DllImport ("WindowObject")]
	private static extern int ShowWindow();

	[DllImport ("WindowObject")]
	private static extern void CloseWindow();

	[DllImport ("WindowObject")]
	private static extern void Blit(IntPtr image, int imageWidth, int imageHeight);

	private Texture2D pBlitTexture;
#endif

	private delegate int LongTimeTask_Delegate();
	LongTimeTask_Delegate d = null;
	IAsyncResult R = null;

	void Start()
	{
#if UNITY_STANDALONE_WIN
		var margins = new MARGINS() { cxLeftWidth = -1 };

		// Get a handle to the window
		var hwnd = GetActiveWindow();

		// Set properties of the window
		// See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633591%28v=vs.85%29.aspx
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

		// Extend the window into the client area
		See: https://msdn.microsoft.com/en-us/library/windows/desktop/aa969512%28v=vs.85%29.aspx 
		DwmExtendFrameIntoClientArea(hwnd, ref margins);
#endif

#if UNITY_STANDALONE_LINUX
		d = new LongTimeTask_Delegate(ShowWindow);
		R = d.BeginInvoke(null, null);
		blit_Texture = Camera.main.targetTexture;
		pBlitTexture = new Texture2D(blit_Texture.width, blit_Texture.height, TextureFormat.ARGB32, false);
		Camera.main.projectionMatrix = Camera.main.projectionMatrix * Matrix4x4.Scale(new Vector3(1.0f, -1.0f, 1.0f));
#endif
	}

	// Pass the output of the camera to the custom material
	// for chroma replacement
	void OnRenderImage(RenderTexture from, RenderTexture to)
	{
		Graphics.Blit(from, to, m_Material);

#if UNITY_STANDALONE_LINUX
		pBlitTexture.ReadPixels(new Rect(0.0f, 0.0f, blit_Texture.width, blit_Texture.height), 0, 0);
		Color32[] texture = pBlitTexture.GetPixels32();
		GCHandle handle = GCHandle.Alloc(texture, GCHandleType.Pinned);
		IntPtr pointer = handle.AddrOfPinnedObject();

		Blit(pointer, pBlitTexture.width, pBlitTexture.height);

		if (handle.IsAllocated)
			handle.Free();
#endif
	}

	void OnApplicationQuit()
	{
#if UNITY_STANDALONE_LINUX
		CloseWindow();
		d.EndInvoke(R);
#endif
	}
}