#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>
#include <iostream>  // Para debugging

namespace render {

  class vector {
  public:
    // --- Constructores ---
    // CORREGIDO: Inicializa m_x, m_y, m_z
    vector() : m_x{0.F}, m_y{0.F}, m_z{0.F} { }

    vector(float cx, float cy, float cz) : m_x{cx}, m_y{cy}, m_z{cz} { }

    // --- Getters ---
    // CORREGIDO: Devuelven la variable miembro 'm_'
    [[nodiscard]] float x() const { return m_x; }

    [[nodiscard]] float y() const { return m_y; }

    [[nodiscard]] float z() const { return m_z; }

    // --- Operadores ---
    // CORREGIDO: Usan m_x, m_y, m_z para acceder a los miembros de 'this'
    vector operator-() const { return {-m_x, -m_y, -m_z}; }

    // CORREGIDO: Usan m_x, m_y, m_z para 'this' y v.x(), v.y(), v.z() para 'v'
    vector operator+(vector const & v) const { return {m_x + v.x(), m_y + v.y(), m_z + v.z()}; }

    vector operator-(vector const & v) const { return {m_x - v.x(), m_y - v.y(), m_z - v.z()}; }

    vector operator*(vector const & v) const { return {m_x * v.x(), m_y * v.y(), m_z * v.z()}; }

    vector operator*(float t) const { return {m_x * t, m_y * t, m_z * t}; }

    vector operator/(float t) const { return {m_x / t, m_y / t, m_z / t}; }

    // CORREGIDO: Usan m_x, m_y, m_z
    vector & operator+=(vector const & v) {
      m_x += v.x();
      m_y += v.y();
      m_z += v.z();
      return *this;
    }

    vector & operator*=(float t) {
      m_x *= t;
      m_y *= t;
      m_z *= t;
      return *this;
    }

    vector & operator/=(float t) { return *this *= (1.F / t); }

    // --- Métodos ---
    // CORREGIDO: Usan m_x, m_y, m_z
    [[nodiscard]] float magnitude() const { return std::sqrtf(m_x * m_x + m_y * m_y + m_z * m_z); }

    [[nodiscard]] float length_squared() const { return m_x * m_x + m_y * m_y + m_z * m_z; }

    [[nodiscard]] vector normalized() const { return *this / magnitude(); }

  private:
    // --- CORREGIDO: Renombradas las variables miembro ---
    float m_x, m_y, m_z;
  };

  // --- Funciones Helpers (Estas ya estaban bien) ---

  inline vector operator*(float t, vector const & v) {
    return {t * v.x(), t * v.y(), t * v.z()};
  }

  inline float dot(vector const & u, vector const & v) {
    return u.x() * v.x() + u.y() * v.y() + u.z() * v.z();
  }

  inline vector cross(vector const & u, vector const & v) {
    return {u.y() * v.z() - u.z() * v.y(), u.z() * v.x() - u.x() * v.z(),
            u.x() * v.y() - u.y() * v.x()};
  }

  inline std::ostream & operator<<(std::ostream & out, vector const & v) {
    return out << '(' << v.x() << ", " << v.y() << ", " << v.z() << ')';
  }

}  // namespace render

#endif
