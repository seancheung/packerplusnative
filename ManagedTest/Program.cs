using System;
using System.Runtime.InteropServices;

namespace ManagedTest
{
    internal class Program
    {
        private static void Main(string[] args)
        {
            int n = 19;
            Texture[] textures = new Texture[n];
            while (n > 0)
            {
                textures[--n] = new Texture {name = n.ToString(), path = (n + 1) + ".png"};
            }

            //Texture[] textures =
            //{
            //    new Texture {name = "1", path = "1.png"},
            //    new Texture {name = "2", path = "2.png"},
            //    new Texture {name = "3", path = "3.png"},
            //    new Texture {name = "4", path = "4.png"},
            //    new Texture {name = "5", path = "5.png"},
            //    new Texture {name = "6", path = "6.png"},
            //    new Texture {name = "7", path = "7.png"},
            //    new Texture {name = "8", path = "8.png"},
            //    new Texture {name = "9", path = "9.png"},
            //    new Texture {name = "10", path = "10.png"},
            //    new Texture {name = "11", path = "11.png"}
            //};

            AtlasPlus atlas = new AtlasPlus();
            Options options = new Options();
            options.crop = true;
            options.algorithm = Algorithm.MaxRects;
            options.maxWidth = 512;
            options.maxHeight = 512;
            options.colorDepth = ColorDepth.TrueColor;
            options.format = Format.PNG;
            options.outputPath = "pack_m.png";
            Options options2 = options;
            options2.outputPath = "pack_t.png";
            options2.algorithm = Algorithm.TightRects;
            Options options3 = options;
            options3.outputPath = "pack_p.png";
            options3.algorithm = Algorithm.Plain;
            Console.ReadLine();
            Pack(textures, atlas, options);
            Pack(textures, atlas, options2);
            Pack(textures, atlas, options3);
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
        private static extern bool Pack([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] Texture[] textures,
            int count, Options option, [Out, MarshalAs(UnmanagedType.LPStr)] out string json, int debug);

        public static void Pack(Texture[] textures, AtlasPlus atlas, Options options, int debug = 0)
        {
            string output;
            Pack(textures, textures.Length, options, out output, debug);

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
            public int maxWidth;
            public int maxHeight;
            [MarshalAs(UnmanagedType.LPWStr)] public string outputPath;
            public ColorDepth colorDepth;
            public Format format;
            public bool crop;
            public Algorithm algorithm;
        }

        public enum Algorithm
        {
            Plain,
            MaxRects,
            TightRects
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