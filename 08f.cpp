#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <random>

using namespace std;

random_device seeder;
const auto seed = seeder.entropy() ? seeder() : time(nullptr);
mt19937 eng(static_cast<mt19937::result_type>(seed));
uniform_real_distribution<float> rand_color(0, 1);
uniform_real_distribution<float> rand_size(0.01f, 0.25f);

//--- �ʿ��� ������� ����
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

GLclampf convert_to_clampf_X(int x, int w) {
	return (x - (w / 2.0f)) / (w / 2.0f);
}
GLclampf convert_to_clampf_Y(int y, int h) {
	return (h / 2.0f - y) / (h / 2.0f);
}

//--- ����� ���� �Լ�
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
void move_shape(int selected, float dx, float dy);

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
bool is_b = false;
GLuint VAO, tri_VBO, Line_VBO;

vector<vector<float>> tri_vertex = {
	{
		-1.0f,0.0f,0.0f,	0.0f,1.0f,0.0f,
		0.0f,0.0f,0.0f,		0.0f,1.0f,0.0f,
		-0.5f,1.0f,0.0f,	0.0f,1.0f,0.0f,
	},

	{
		1.0f,0.0f,0.0f,	1.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,		1.0f,0.0f,0.0f,
		0.5f,1.0f,0.0f,	1.0f,0.0f,0.0f,
	},

	{
		-1.0f,0.0f,0.0f,	0.0f,0.0f,1.0f,
		0.0f,0.0f,0.0f,		0.0f,0.0f,1.0f,
		-0.5f,-1.0f,0.0f,	0.0f,0.0f,1.0f,
	},

	{
		1.0f,0.0f,0.0f,	0.5f,0.5f,0.5f,
		0.0f,0.0f,0.0f,		0.5f,0.5f,0.5f,
		0.5f,-1.0f,0.0f,	0.5f,0.5f,0.5f,
	}
};

vector<float> Lines = {
	-1.0f,0.0f,0.0f,	0.0f,0.0f,0.0f,
	1.0f,0.0f,0.0f,		0.0f,0.0f,0.0f,
	0.0f,-1.0f,0.0f,	0.0f,0.0f,0.0f,
	0.0f,1.0f,0.0f,		0.0f,0.0f,0.0f
};


GLvoid InitBuffer()
{
	//--- VAO�� VBO ��ü ����
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &tri_VBO);
	glGenBuffers(1, &Line_VBO);
}

//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	width = 800;
	height = 600;
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();

	//--- VAO/VBO ���� �ʱ�ȭ
	shaderProgramID = make_shaderProgram();
	InitBuffer();

	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutMainLoop();
}

void make_vertexShaders()
{
	GLchar* vertexSource;

	vertexSource = filetobuf("vertex.glsl");

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment.glsl"); // �����׼��̴� �о����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	make_vertexShaders();
	make_fragmentShaders();
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
	glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
	glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�

	glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�

	return shaderID;
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	GLfloat rColor, gColor, bColor;
	vector<float> triangle;
	rColor = 1.0;
	gColor = 1.0;
	bColor = 1.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);
	if (!Lines.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, Line_VBO);
		glBufferData(GL_ARRAY_BUFFER, Lines.size() * sizeof(float), Lines.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_LINES, 0, Lines.size() / 6);
		// �� ���� XYZ ��ǥ
	}

	if (!tri_vertex.empty()) {
		for (int i = 0; i < 4; i++)
		{
			if (is_b)
			{
				for (int j = 0; j < tri_vertex[i].size()/18; j++)
				{
					triangle.clear();
					for (int k = 0; k < 18; k++)
					{
						triangle.push_back(tri_vertex[i][(18 * j) + k]);
					}
					glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
					glBufferData(GL_ARRAY_BUFFER, triangle.size() * sizeof(float), triangle.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
					glEnableVertexAttribArray(0);
					glEnableVertexAttribArray(1);

					glDrawArrays(GL_LINE_LOOP, 0, triangle.size() / 6);
				}
			}
			else
			{
				glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
				glBufferData(GL_ARRAY_BUFFER, tri_vertex[i].size() * sizeof(float), tri_vertex[i].data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);

				glDrawArrays(GL_TRIANGLES, 0, tri_vertex[i].size() / 6);
			}
		}
		// �� ���� XYZ ��ǥ
	}


	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		is_b = false;
		break;
	case 'b':
		is_b = true;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void Make_triangle(int index,float cx,float cy,bool clear) {
	float ds = rand_size(eng);
	float tri_r = rand_color(eng);
	float tri_g = rand_color(eng);
	float tri_b = rand_color(eng);

	if (clear)
	{
		tri_vertex[index].clear();
		tri_vertex[index].push_back(cx - (ds / 2));
		tri_vertex[index].push_back(cy - ds);
		tri_vertex[index].push_back(0.0f);
		tri_vertex[index].push_back(tri_r);
		tri_vertex[index].push_back(tri_g);
		tri_vertex[index].push_back(tri_b);

		tri_vertex[index].push_back(cx + (ds / 2));
		tri_vertex[index].push_back(cy - ds);
		tri_vertex[index].push_back(0.0f);
		tri_vertex[index].push_back(tri_r);
		tri_vertex[index].push_back(tri_g);
		tri_vertex[index].push_back(tri_b);

		tri_vertex[index].push_back(cx);
		tri_vertex[index].push_back(cy + ds);
		tri_vertex[index].push_back(0.0f);
		tri_vertex[index].push_back(tri_r);
		tri_vertex[index].push_back(tri_g);
		tri_vertex[index].push_back(tri_b);
	}
	else
	{
		if (!((tri_vertex[index].size() / 18) > 2)) {
			tri_vertex[index].push_back(cx - (ds / 2));
			tri_vertex[index].push_back(cy - ds);
			tri_vertex[index].push_back(0.0f);
			tri_vertex[index].push_back(tri_r);
			tri_vertex[index].push_back(tri_g);
			tri_vertex[index].push_back(tri_b);

			tri_vertex[index].push_back(cx + (ds / 2));
			tri_vertex[index].push_back(cy - ds);
			tri_vertex[index].push_back(0.0f);
			tri_vertex[index].push_back(tri_r);
			tri_vertex[index].push_back(tri_g);
			tri_vertex[index].push_back(tri_b);

			tri_vertex[index].push_back(cx);
			tri_vertex[index].push_back(cy + ds);
			tri_vertex[index].push_back(0.0f);
			tri_vertex[index].push_back(tri_r);
			tri_vertex[index].push_back(tri_g);
			tri_vertex[index].push_back(tri_b);
		}
	}
}

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, width);
	GLclampf cy = convert_to_clampf_Y(y, height);
	if (state == GLUT_DOWN) {
		if (cx < 0 && cy >= 0) // ����
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(0, cx, cy, true);
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				Make_triangle(0, cx, cy, false);
			}
		}
		else if (cx >= 0 && cy >= 0) //����
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(1, cx, cy, true);
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				Make_triangle(1, cx, cy, false);
			}
		}
		else if (cx < 0 && cy < 0) //�޾�
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(2, cx, cy, true);
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				Make_triangle(2, cx, cy, false);
			}
		}
		else if (cx >= 0 && cy < 0) //����
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(3, cx, cy, true);
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				Make_triangle(3, cx, cy, false);
			}
		}
	}
	glutPostRedisplay();
}