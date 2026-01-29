
#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

using namespace std;

class Texture {
    static uint currentPos;

    uint texId;
    uint texPos;

public:
    Texture(): texId(0), texPos(0) {}

    Texture(const char *path): texId(0), texPos(0) {
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, nrChannels;
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

        if (data != nullptr) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                        GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            cout << "Failed to load texture" << endl;
        }

        texPos = currentPos;
        currentPos++;

        stbi_image_free(data);
    }

    uint getId() const {
        return texId;
    }

    void bind(GLuint programId, const char *name) const {
        glActiveTexture(GL_TEXTURE0 + this->texPos);
        glBindTexture(GL_TEXTURE_2D, this->texId);
        glUniform1i(glGetUniformLocation(programId, name), texPos);
    }
};

uint Texture::currentPos = 0;