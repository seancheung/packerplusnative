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
                new Texture {name = "1", path = "1.png"},
                new Texture {name = "2", path = "2.png"},
                new Texture {name = "3", path = "3.png"},
                new Texture {name = "4", path = "4.png"},
                new Texture {name = "5", path = "5.png"},
                new Texture {name = "6", path = "6.png"},
                new Texture {name = "7", path = "7.png"},
                new Texture {name = "8", path = "8.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"},
                new Texture {name = "9", path = "9.png"}
            };

            AtlasPlus atlas = new AtlasPlus();
            Options options = new Options();
            options.crop = true;
            options.algorithm = Algorithm.MaxRects;
            Console.ReadLine();
            Pack(textures, atlas, 1024, 1024, ColorDepth.TrueColor, Format.PNG, "pack.png", options);
            //Console.WriteLine(atlas);
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
            [Out, MarshalAs(UnmanagedType.LPStr)] out string json, Options option, int debug);

        public static void Pack(Texture[] textures, AtlasPlus atlas, int width, int height, ColorDepth depth,
            Format format, string path, Options options, int debug = 0)
        {
            string output;
            Pack(textures, textures.Length, width, height, path, depth, format, out output, options, debug);

            Console.WriteLine(output);
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
            public int width;
            public int height;
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

        [StructLayout(LayoutKind.Sequential)]
        public struct Options
        {
            public bool crop;
            public Algorithm algorithm;
        }

        public enum Algorithm
        {
            Plain,
            MaxRects
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