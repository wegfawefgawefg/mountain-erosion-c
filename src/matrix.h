#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

// Set the 4x4 matrix 'm' to the identity matrix.
static void mat4_identity(float *m)
{
    for (int i = 0; i < 16; i++)
        m[i] = 0.0f;
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
}

// Create a perspective projection matrix.
// fov: field of view in radians, aspect: width/height,
// near and far: clipping planes.
static void mat4_perspective(float *m, float fov, float aspect, float near, float far)
{
    float f = 1.0f / tanf(fov * 0.5f);
    m[0] = f / aspect;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = f;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = (2.0f * far * near) / (near - far);
    m[15] = 0.0f;
}

// Multiply two 4x4 matrices: result = a * b, with matrices in column-major order.
static void mat4_mul(float *result, const float *a, const float *b)
{
    float temp[16];
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            temp[col * 4 + row] = 0.0f;
            for (int i = 0; i < 4; i++)
            {
                temp[col * 4 + row] += a[i * 4 + row] * b[col * 4 + i];
            }
        }
    }
    for (int i = 0; i < 16; i++)
        result[i] = temp[i];
}

// Create a rotation matrix around the X axis.
static void mat4_rotate_x(float *m, float angle)
{
    mat4_identity(m);
    m[5] = cosf(angle);
    m[6] = -sinf(angle);
    m[9] = sinf(angle);
    m[10] = cosf(angle);
}

// Create a rotation matrix around the Y axis.
static void mat4_rotate_y(float *m, float angle)
{
    mat4_identity(m);
    m[0] = cosf(angle);
    m[2] = -sinf(angle);
    m[8] = sinf(angle);
    m[10] = cosf(angle);
}

// Create a translation matrix.
static void mat4_translate(float *m, float x, float y, float z)
{
    mat4_identity(m);
    m[12] = x;
    m[13] = y;
    m[14] = z;
}

static void mat4_lookAt(float *m, const float *eye, const float *center, const float *up)
{
    float f[3] = {center[0] - eye[0], center[1] - eye[1], center[2] - eye[2]};
    // Normalize f.
    float fMag = sqrtf(f[0] * f[0] + f[1] * f[1] + f[2] * f[2]);
    f[0] /= fMag;
    f[1] /= fMag;
    f[2] /= fMag;

    float s[3] = {
        f[1] * up[2] - f[2] * up[1],
        f[2] * up[0] - f[0] * up[2],
        f[0] * up[1] - f[1] * up[0]};
    float sMag = sqrtf(s[0] * s[0] + s[1] * s[1] + s[2] * s[2]);
    s[0] /= sMag;
    s[1] /= sMag;
    s[2] /= sMag;

    float u[3] = {
        s[1] * f[2] - s[2] * f[1],
        s[2] * f[0] - s[0] * f[2],
        s[0] * f[1] - s[1] * f[0]};

    // Column-major order.
    m[0] = s[0];
    m[1] = u[0];
    m[2] = -f[0];
    m[3] = 0.0f;

    m[4] = s[1];
    m[5] = u[1];
    m[6] = -f[1];
    m[7] = 0.0f;

    m[8] = s[2];
    m[9] = u[2];
    m[10] = -f[2];
    m[11] = 0.0f;

    m[12] = -(s[0] * eye[0] + s[1] * eye[1] + s[2] * eye[2]);
    m[13] = -(u[0] * eye[0] + u[1] * eye[1] + u[2] * eye[2]);
    m[14] = (f[0] * eye[0] + f[1] * eye[1] + f[2] * eye[2]);
    m[15] = 1.0f;
}

#endif // MATRIX_H
