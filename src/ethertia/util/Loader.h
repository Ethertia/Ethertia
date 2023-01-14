//
// Created by Dreamtowards on 2022/3/30.
//

#ifndef ETHERTIA_LOADER_H
#define ETHERTIA_LOADER_H

#include <initializer_list>
#include <utility>
#include <fstream>
#include <array>
#include <filesystem>
#include <span>

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <tinyfd/tinyfiledialogs.h>

#include <ethertia/render/Texture.h>
#include <ethertia/util/BitmapImage.h>
#include <ethertia/render/VertexBuffer.h>
#include <ethertia/render/Model.h>
#include <ethertia/util/OBJLoader.h>
#include <ethertia/audio/AudioEngine.h>


class Loader {

public:

    inline static std::string ASSETS = "assets/";


    static void showMessageBox(const char* title, const char* message) {
        tinyfd_messageBox(title, message, "ok", "question", 1);
    }
    static std::string showInputBox(const char* title, const char* message, const char* def) {
        return tinyfd_inputBox(title, message, def);  // free()?
    }

    // File, Folder, URL
    static void openURL(const std::string& url) {
        const char* cmd = nullptr;
#if _WIN32
        cmd = "start ";  // windows
#elif __APPLE__
        cmd = "open ";  // macos
#elif __unix__
        cmd = "xdg-open ";  // linux
#else
        assert(false);  // Not supported OS yet.
#endif
        std::system(std::string(cmd + url).c_str());
    }

    static const char* system() {
#if __WIN32__
        return "WINDOWS";
#elif __APPLE__
        return "DARWIN";
#elif __unix__
        return "LINUX";
#else
        return "_UNKNOWN";
#endif
    }

    static std::pair<char*, size_t> loadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios_base::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed open file. "+path);
            //Log::warn("Failed open file: ", path);
            //return std::make_pair(nullptr, -1);
        }
        file.seekg(0, std::ios_base::end);
        size_t len = file.tellg();
        file.seekg(0, std::ios_base::beg);

        char* buf = new char[len];
        file.read(buf, len);
        file.close();

        return std::pair(buf, len);
    }
    static bool fileExists(std::string_view path) {
        return std::filesystem::exists(path);
    }
    inline static std::string assetsFile(const std::string& p) {
        return ASSETS + p;
    }

    static std::pair<char*, size_t> loadAssets(const std::string& p) {
        return loadFile(assetsFile(p));
    }

    static std::string loadAssetsStr(const std::string& p) {
        auto m = loadAssets(p);
        return std::string(m.first, m.second);
    }
    static std::string loadFileStr(const std::string& p) {
        auto m = loadFile(p);
        return std::string(m.first, m.second);
    }

    static VertexBuffer* loadOBJ(const std::string& objstr) {
        VertexBuffer* vbuf = new VertexBuffer();
        std::stringstream ss(objstr);
        OBJLoader::loadOBJ(ss, vbuf);
        return vbuf;
    }

    static void saveOBJ(const std::string& filename, size_t verts, float* pos, float* uv =nullptr, float* norm =nullptr) {
        std::stringstream ss;
        OBJLoader::saveOBJ(ss, verts, pos, uv, norm);

        ensureFileParentDirsReady(filename);
        std::ofstream fs(filename);
        fs << ss.str();
        fs.close();
    }

    static int16_t* loadOGG(std::pair<char*, size_t> data, size_t* dst_len, int* dst_channels, int* dst_sampleRate);


    static AudioBuffer* loadOGG(std::pair<char*, size_t> data) {
        size_t len;
        int channels;
        int sampleRate = 0;

        int16_t* pcm = Loader::loadOGG(data, &len, &channels, &sampleRate);
        ALuint format = channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;

        AudioBuffer* buf = new AudioBuffer();
        buf->buffer_data(format, pcm, len, sampleRate);
        return buf;
    }

    // PCM, 16-bit sample, 1-channel
    static void saveWAV(const void* pcm, size_t size, std::ostream& dst, int samplePerSec = 44100) {
        // endianness problem. may cause wrong on big-endian system.
        struct WAV_HEADER {
            /* RIFF Chunk Descriptor */
            uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
            uint32_t ChunkSize;                     // RIFF Chunk Size
            uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
            /* "fmt" sub-chunk */
            uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
            uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
            uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
            // Mu-Law, 258=IBM A-Law, 259=ADPCM
            uint16_t NumOfChan = 1;   // Number of channels 1=Mono 2=Sterio
            uint32_t SamplesPerSec = 16000;   // Sampling Frequency in Hz
            uint32_t bytesPerSec = 16000 * 2; // bytes per second
            uint16_t blockAlign = 2;          // 2=16-bit mono, 4=16-bit stereo
            uint16_t bitsPerSample = 16;      // Number of bits per sample
            /* "data" sub-chunk */
            uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
            uint32_t Subchunk2Size;                        // Sampled data length
        };
        static_assert(sizeof(WAV_HEADER) == 44);

        WAV_HEADER hdr;
        hdr.ChunkSize = size + sizeof(WAV_HEADER) - 8;
        hdr.Subchunk2Size = size;

        hdr.SamplesPerSec = samplePerSec;
        hdr.bytesPerSec = samplePerSec * 2;  // 16bit sample.

        dst.write(reinterpret_cast<const char*>(&hdr), sizeof(WAV_HEADER));

        dst.write((char*)pcm, size);
    }

    static BitmapImage* loadPNG(const void* data, u32 len) {
        int width, height, channels;
        void* pixels = stbi_load_from_memory((unsigned char*)data, len, &width, &height, &channels, 4);
        return new BitmapImage(width, height, (unsigned int*)pixels);
    }
    static BitmapImage* loadPNG(std::pair<void*, u32> m) {
        return loadPNG(m.first, m.second);
    }

    static void savePNG(BitmapImage* img, const std::string& filename) {
        ensureFileParentDirsReady(filename);
        if (!stbi_write_png(filename.c_str(), img->getWidth(), img->getHeight(), 4, img->getPixels(), 0)) {
            throw std::runtime_error("Failed to write PNG. "+filename);
        }
    }

    static void ensureFileParentDirsReady(const std::string& filename) {
        // mkdirs for parents of the file.
        int _dir = filename.rfind('/');
        if (_dir != std::string::npos) {
            std::filesystem::create_directories(filename.substr(0, _dir));
        }
    }

    static Model* loadModel(size_t vcount, const std::vector<std::pair<int, float*>>& vdats) {
        u32 vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        Model* m = new Model(vao, vcount);

        int i = 0;
        for (auto vd : vdats) {
            int vlen = vd.first;
            float* vdat = vd.second;

            u32 vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vlen*vcount, vdat, GL_STATIC_DRAW);

            glVertexAttribPointer(i, vlen, GL_FLOAT, false, 0, nullptr);
            glEnableVertexAttribArray(i);
            m->vbos.push_back(vbo);
            i++;
        }
        return m;
    }
    static Model* loadModel(VertexBuffer* vbuf) {
        std::vector<std::pair<int, float*>> ls;
        ls.emplace_back(3, &vbuf->positions[0]);
        ls.emplace_back(2, &vbuf->textureCoords[0]);
        ls.emplace_back(3, &vbuf->normals[0]);

        return loadModel(vbuf->vertexCount(), ls);
    }
    static Model* loadModel(size_t vcount, std::initializer_list<std::pair<int, float*>> vdats) {
        return loadModel(vcount, std::vector(vdats));
    }

    static Texture* loadTexture(const std::string& assets_p) {
        return Loader::loadTexture(Loader::loadPNG(Loader::loadAssets(assets_p)));
    }
    static Texture* loadTexture(BitmapImage* img) {
        std::unique_ptr<std::uint32_t> pixels(new std::uint32_t[img->getWidth() * img->getHeight()]);
        img->getVerticalFlippedPixels(pixels.get());

        return Loader::loadTexture(img->getWidth(), img->getHeight(), pixels.get());
    }

    /// pixels_VertInv: need y/vertical flipped pixels. cause of GL feature.
    static Texture* loadTexture(int w, int h, u32* pixels_VertFlip, int intlfmt = GL_RGBA, int fmt = GL_RGBA, int type = GL_UNSIGNED_BYTE) {
        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        auto* tex = new Texture(texId, w, h);

        glTexImage2D(GL_TEXTURE_2D, 0, intlfmt, w, h, 0, fmt, type, pixels_VertFlip);
        // glTexSubImage2D();


        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.2f);
//        if (GL.getCapabilities().GL_EXT_texture_filter_anisotropic) {
//            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0);  // set 0 if use TextureFilterAnisotropic
//            float amount = Math.min(4f, glGetFloat(EXTTextureFilterAnisotropic.GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT));
//            glTexParameterf(target, EXTTextureFilterAnisotropic.GL_TEXTURE_MAX_ANISOTROPY_EXT, amount);
//            LOGGER.info("ENABLED GL_EXT_texture_filter_anisotropic");
//         }

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //GL_LINEAR, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST

        glGenerateMipmap(GL_TEXTURE_2D);

        return tex;
    }

    // imgs order: Right, Left, Top, Bottom, Front, Back.
    static Texture* loadCubeMap(std::vector<BitmapImage*> imgs) {
        assert(imgs.size() == 6);
        int w = imgs[0]->getWidth();
        int h = imgs[0]->getHeight();

        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
        Texture* tex = new Texture(texId, w, h);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        for (int i = 0; i < 6; ++i) {
            BitmapImage* img = imgs[i];
            assert(img->getWidth() == w && img->getHeight() == h);

            // flipped y.
            void* pixels = img->getPixels();

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        }

        return tex;
    }

};

#endif //ETHERTIA_LOADER_H
