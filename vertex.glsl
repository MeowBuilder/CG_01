#version 330 core

in vec3 vPos;
in vec3 vColor;
out vec3 passColor;

void main(void)
{
	gl_Position = vec4 (vPos.x, vPos.y, vPos.z, 1.0);
	passColor = vColor;
}