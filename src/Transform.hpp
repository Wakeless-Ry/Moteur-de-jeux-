#ifndef TRANSFORM
#define TRANSFORM

#include "glm/detail/type_mat.hpp"
#include <glm/ext.hpp>

class Transform {
    glm::mat3 m_rss_matrix;
    glm::vec3 m_translation_vec;

  public:
    Transform() : m_rss_matrix(1.), m_translation_vec(0.) {}

    inline glm::vec3 getTranslationVector() const {
        return this->m_translation_vec;
    }

    inline glm::mat3 getRSSMatrix() const { return this->m_rss_matrix; }

    inline Transform &scale(float x, float y, float z) {
        glm::mat3 s(x, 0., 0., 0., y, 0., 0., 0., z);
        this->m_rss_matrix *= s;
        return *this;
    }

    inline Transform &scale(float value) {
        this->scale(value, value, value);
        return *this;
    }
    inline Transform &scale(const glm::vec3 &s) {
        this->scale(s[0], s[1], s[2]);
        return *this;
    }

    inline Transform &skew(const glm::mat3 &skewMatrix) {
        this->m_rss_matrix *= skewMatrix;
        return *this;
    }

    inline Transform &translate(float x, float y, float z) {
        glm::vec3 t(x, y, z);
        this->m_translation_vec += t;
        return *this;
    }

    inline Transform &translate(float value) {
        this->translate(value, value, value);
        return *this;
    }

    inline Transform &translate(const glm::vec3 &s) {
        this->translate(s[0], s[1], s[2]);
        return *this;
    }

    inline Transform &rotationX(float angle) {
        glm::mat3 r(1, 0., 0., 0., cos(glm::radians(angle)),
                    -sin((glm::radians(angle))), 0., sin((glm::radians(angle))),
                    cos(glm::radians(angle)));
        this->m_rss_matrix *= r;
        return *this;
    }

    inline Transform &rotationY(float angle) {
        glm::mat3 r(cos(glm::radians(angle)), 0., sin((glm::radians(angle))),
                    0., 1., 0., -sin((glm::radians(angle))), 0.,
                    cos(glm::radians(angle)));
        this->m_rss_matrix *= r;
        return *this;
    }

    inline Transform &rotationZ(float angle) {
        glm::mat3 r(cos(glm::radians(angle)), -sin((glm::radians(angle))), 0.,
                    sin((glm::radians(angle))), cos(glm::radians(angle)), 0.,
                    0., 0., 1.);
        this->m_rss_matrix *= r;
        return *this;
    }

    inline Transform &rotation(float x, float y, float z) {
        this->rotationX(x);
        this->rotationY(y);
        this->rotationZ(z);
        return *this;
    }

    inline Transform &rotation(const glm::vec3 &r) {
        this->rotationX(r[0]);
        this->rotationY(r[1]);
        this->rotationZ(r[2]);
        return *this;
    }

    inline Transform &transform(const Transform &transform) {
        this->m_rss_matrix *= transform.m_rss_matrix;
        this->m_translation_vec += transform.m_translation_vec;
        return *this;
    }

    inline glm::vec4 computeVec4(const glm::vec4 &in) const {
        return computeTransformMatrix() * in;
    }

    inline glm::vec3 computeVec3(const glm::vec3 &in) const {
        glm::vec4 tmp = computeTransformMatrix() * glm::vec4(in, 1.);
        return glm::vec3(tmp[0], tmp[1], tmp[2]);
    }

    inline glm::mat4 computeMat4(const glm::mat4 &in) const {
        return computeTransformMatrix() * in;
    }

    inline glm::mat4 computeTransformMatrix() const {
        return glm::mat4(
            m_rss_matrix[0][0], m_rss_matrix[1][0], m_rss_matrix[2][0],
            m_translation_vec[0], m_rss_matrix[0][1], m_rss_matrix[1][1],
            m_rss_matrix[2][1], m_translation_vec[1], m_rss_matrix[0][2],
            m_rss_matrix[1][2], m_rss_matrix[2][2], m_translation_vec[2], 0.,
            0., 0., 1.);
    }
};

#endif // TRANSFORM