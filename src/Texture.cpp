#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <lib/stb_image.h>

#include "Texture.h"

Texture::Texture() : texId(0) {}

Texture::Texture(const char *path, int pos) : texId(0), pos(pos) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data != nullptr) {
        glGenTextures(1, &texId);
        glActiveTexture(GL_TEXTURE0 + this->pos);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else
            format = GL_RGBA;

        GLenum internalFormat = format;
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
}

uint Texture::getId() const { return this->texId; }

void Texture::bind(GLuint programId, const char *name) const {
    glUseProgram(programId);
    glActiveTexture(GL_TEXTURE0 + this->pos);
    glBindTexture(GL_TEXTURE_2D, this->texId);
    glUniform1i(glGetUniformLocation(programId, name), this->pos);
}

void Texture::cleanUp() {
    if (this->texId != 0) {
        glDeleteTextures(1, &this->texId);
        this->texId = 0;
    }
}