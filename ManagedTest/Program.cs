using System;
using System.Runtime.InteropServices;

namespace ManagedTest
{
	internal class Program
	{
		private static void Main(string[] args)
		{
			Texture[] textures =
			{
				new Texture {name = "background_1", path = "background_1.png"},
				new Texture {name = "background_2", path = "background_2.png"},
				new Texture {name = "background_3", path = "background_3.png"},
				new Texture {name = "background_4", path = "background_4.png"},
				new Texture {name = "background_5", path = "background_5.png"}
			};

			AtlasPlus atlas = new AtlasPlus();
			Console.ReadLine();
			Pack(textures, atlas, 1024, 1024, ColorDepth.TrueColor, Format.PNG, "pack.png");
			Console.WriteLine(atlas);
			//Create(1024, 1024, "empty.png", ColorDepth.TrueColor, Format.PNG, new Color(100, 250, 200, 255));
			Console.ReadLine();
		}

		[DllImport("PackerPlus", EntryPoint = "release")]
		private static extern void Release(IntPtr ptr);

		[DllImport("PackerPlus", EntryPoint = "create_empty")]
		private static extern void Create(int width, int height, [MarshalAs(UnmanagedType.LPWStr)] string path,
			ColorDepth depth, Format format, Color color);

		[DllImport("PackerPlus", EntryPoint = "pack")]
		private static extern bool Pack([In, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] Texture[] textures,
			[In] int count, [In] int maxWidth, int maxHeight, [In, MarshalAs(UnmanagedType.LPWStr)] string path,
			[In] ColorDepth depth, [In] Format format,
			[Out] out int textureCount, [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 7)] out Texture[] atlasTextures,
			[Out] out int spriteCount, [Out, MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 9)] out Sprite[] sprites);

		public static void Pack(Texture[] textures, AtlasPlus atlas, int width, int height, ColorDepth depth,
			Format format, string path)
		{
			var count = textures.Length;
			//var size = new Size {width = width, height = height};
			Atlas info = new Atlas();
			Texture[] atlasTextures;
			Sprite[] sprites;
			int textureCount;
			int spriteCount;


			Pack(textures, textures.Length, width, height, path, depth, format, out textureCount, out atlasTextures,
				out spriteCount,
				out sprites);

			//atlas.maxWidth = info.maxSize.width;
			//atlas.maxHeight = info.maxSize.height;
			//atlas.textures = new TextureInfo[info.textureCount];
			//for (int i = 0; i < info.textureCount; i++)
			//{
			//	atlas.textures[i] = new TextureInfo
			//	{
			//		width = info.textures[i].size.width,
			//		height = info.textures[i].size.height,
			//		texture = info.textures[i].path
			//	};
			//}
			//atlas.sprites = new SpriteInfo[info.spriteCount];
			//for (int i = 0; i < info.spriteCount; i++)
			//{
			//	atlas.sprites[i] = new SpriteInfo
			//	{
			//		name = info.sprites[i].name,
			//		section = info.sprites[i].section,
			//		//sourceRect = new Rect(, 0, info.sprites[i].size.width, info.sprites[i].size.height),
			//		uvRect =
			//			Rect.MinMaxRect(info.sprites[i].uv.xMin, info.sprites[i].uv.yMin, info.sprites[i].uv.xMax,
			//				info.sprites[i].uv.yMax)
			//	};
			//}
		}

		#region Marshal

		[StructLayout(LayoutKind.Sequential)]
		private struct UVRect
		{
			public float xMin;
			public float yMin;
			public float xMax;
			public float yMax;
		}

		[StructLayout(LayoutKind.Sequential)]
		private struct TileRect
		{
			public int xMin;
			public int yMin;
			public int xMax;
			public int yMax;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Texture
		{
			[MarshalAs(UnmanagedType.LPWStr)] public string path;
			[MarshalAs(UnmanagedType.LPStr)] public string name;
		}

		[StructLayout(LayoutKind.Sequential)]
		private struct Sprite
		{
			[MarshalAs(UnmanagedType.LPStr)] public string name;
			public UVRect uv;
			public TileRect rect;
			public int section;
		}

		private struct Atlas
		{
			public int textureCount;
			public Texture[] textures;
			public int spriteCount;
			public Sprite[] sprites;
		}

		[StructLayout(LayoutKind.Sequential)]
		public struct Color
		{
			public byte r;
			public byte g;
			public byte b;
			public byte a;

			public Color(byte r, byte g, byte b, byte a)
			{
				this.r = r;
				this.g = g;
				this.b = b;
				this.a = a;
			}
		}

		public enum ColorDepth
		{
			One = 1,
			Four = 4,
			Eight = 8,
			TrueColor = 24
		}

		public enum Format
		{
			BMP = 1,
			GIF,
			JPG,
			PNG,
			ICO,
			TIF,
			TGA,
			PCX,
			WBMP,
			WMF
		}

		#endregion

		public class AtlasPlus
		{
			public TextureInfo[] textures;
			public SpriteInfo[] sprites;
			public int maxWidth;
			public int maxHeight;
		}

		[Serializable]
		public class SpriteInfo
		{
			public string name;
			public Rect uvRect;
			public Rect sourceRect;
			public int section;
		}

		[Serializable]
		public class TextureInfo
		{
			public string texture;
			public int width;
			public int height;
		}

		public class Rect
		{
			public Rect(float left, float top, float right, float bottom)
			{
			}

			public static Rect MinMaxRect(float xMin, float yMin, float xMax, float yMax)
			{
				return new Rect(xMin, yMin, xMax, yMax);
			}
		}
	}
}