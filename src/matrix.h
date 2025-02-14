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

// Multiply two 4x4 matrices: result = a * b.
static void mat4_mul(float *result, const float *a, const float *b)
{
    float temp[16];
    for (int row = 0; row < 4; row++)
    {
        for (int col = 0; col < 4; col++)
        {
            temp[row * 4 + col] = 0.0f;
            for (int i = 0; i < 4; i++)
            {
                temp[row * 4 + col] += a[row * 4 + i] * b[i * 4 + col];
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
    m[2] = sinf(angle);
    m[8] = -sinf(angle);
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

#endif // MATRIX_H
