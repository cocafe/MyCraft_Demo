#version 330 core

// Parameters transfered by glEnableVertexAttribArray()
layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 vertex_uv;

// UV data needed by fragment shader
out vec2 uv;

// Perspective transform matrix
uniform mat4 mat_transform;

// Camera Position
uniform vec3 camera;

// For fog mixing
uniform float fog_distance;
out float fog_factor;

void main() {
    gl_Position = mat_transform * vec4(vertex_position, 1);

    uv = vertex_uv;

    // Fog
    float camera_distance = distance(camera, vertex_position);
    fog_factor = pow(clamp(camera_distance / fog_distance, 0.0, 1.0), 4.0);
}