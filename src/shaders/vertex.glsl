#version 330 core

in vec3 position;
in vec3 normal;
in vec2 texturecoords;

uniform mat4 view;
uniform mat4 model;

out vec3 fragpos;
out vec3 fragnormal;
out vec2 fragtexturecoords;

void main(){
    fragpos = vec3(model * vec4(position, 1.0));
    fragnormal = mat3(transpose(inverse(model))) * normal;
    fragtexturecoords = texturecoords;

    gl_Position = view * vec4(fragpos, 1.0);
}
