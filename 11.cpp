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
void TimerFunction(int value);
void change_shape(int index);

//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü
GLuint VAO, VBO, Line_VBO;

vector<vector<float>> vertexs = {
	{
		-0.75,0.25,0.0,		0.5,0.4,1.0,
		-0.25,0.75,0.0,		0.5,0.4,1.0,
	},

	{
		0.25,0.25,0.0,		0.5,0.4,1.0,
		0.5,0.75,0.0,		0.5,0.4,1.0,
		0.75,0.25,0.0,		0.5,0.4,1.0,
	},

	{
		-0.75,-0.75,0.0,	0.5,0.4,1.0,
		-0.75,-0.25,0.0,	0.5,0.4,1.0,
		-0.25,-0.75,0.0,	0.5,0.4,1.0,

		-0.75,-0.25,0.0,	0.5,0.4,1.0,
		-0.25,-0.25,0.0,	0.5,0.4,1.0,
		-0.25,-0.75,0.0,	0.5,0.4,1.0,
	},

	{
		0.25,-0.45,0.0,		0.5,0.4,1.0,
		0.50,-0.25,0.0,		0.5,0.4,1.0,
		0.30,-0.75,0.0,		0.5,0.4,1.0,

		0.50,-0.25,0.0,		0.5,0.4,1.0,
		0.75,-0.45,0.0,		0.5,0.4,1.0,
		0.70,-0.75,0.0,		0.5,0.4,1.0,

		0.30,-0.75,0.0,		0.5,0.4,1.0,
		0.50,-0.25,0.0,		0.5,0.4,1.0,
		0.70,-0.75,0.0,		0.5,0.4,1.0,
	},

	{

	}
};
vector<int> cur_shape = { 0,1,2,3, 0 };

vector<vector<float>> goal = { {},{},{},{},{} };

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

	glGenBuffers(1, &VBO);
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

	glutTimerFunc(10, TimerFunction, 0);

	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
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

	if (vertexs[4].size() == 0)
	{
		if (!Lines.empty()) {
			glBindBuffer(GL_ARRAY_BUFFER, Line_VBO);
			glBufferData(GL_ARRAY_BUFFER, Lines.size() * sizeof(float), Lines.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glDrawArrays(GL_LINES, 0, Lines.size() / 6);
		}

		if (!vertexs.empty()) {
			for (int i = 0; i < 4; i++)
			{
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, vertexs[i].size() * sizeof(float), vertexs[i].data(), GL_STATIC_DRAW);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);

				if (cur_shape[i] == 0)
				{
					glDrawArrays(GL_LINE_LOOP, 0, vertexs[i].size() / 6);
				}
				else
				{
					glDrawArrays(GL_TRIANGLES, 0, vertexs[i].size() / 6);
				}
			}
		}
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertexs[4].size() * sizeof(float), vertexs[4].data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		if (cur_shape[4] == 0)
		{
			glDrawArrays(GL_LINE_LOOP, 0, vertexs[4].size() / 6);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, vertexs[4].size() / 6);
		}
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
	case 'l':
		vertexs[4].clear();
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);
		cur_shape[4] = 0;
		break;
	case 't':
		vertexs[4].clear();
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0);
		vertexs[4].push_back(0.75);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);
		cur_shape[4] = 1;
		break;
	case 'r':
		vertexs[4].clear();
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		cur_shape[4] = 2;
		break;
	case 'p':
		vertexs[4].clear();
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.05);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(-0.20);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.05);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.20);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(-0.20);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0);
		vertexs[4].push_back(0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		vertexs[4].push_back(0.20);
		vertexs[4].push_back(-0.25);
		vertexs[4].push_back(0.0);
		vertexs[4].push_back(0.5);
		vertexs[4].push_back(0.4);
		vertexs[4].push_back(1.0);

		cur_shape[4] = 3;
		break;
	case 'a':
		vertexs[4].clear();
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

bool is_same_point(float this_x, float this_y, float goal_x, float goal_y) {
	return sqrt((this_x - goal_x) * (this_x - goal_x) + (this_y - goal_y) * (this_y - goal_y)) < 0.01f;
}

void change_shape(int index) {
	float mid_x , mid_y;

	switch (cur_shape[index])
	{
	case 0: // �� -> �ﰢ��
		if (vertexs[index].size() < 18)
		{
			goal[index].clear();
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);
			vertexs[index].push_back(vertexs[index][vertexs[index].size() - 6]);

			mid_x = (vertexs[index][(0 * 6)] + vertexs[index][(1 * 6)]) / 2;
			mid_y = (vertexs[index][(0 * 6) + 1] + vertexs[index][(1 * 6) + 1]) / 2;

			goal[index].push_back(mid_x);
			goal[index].push_back(mid_y + 0.25f);

			goal[index].push_back(mid_x + 0.25f);
			goal[index].push_back(mid_y - 0.25f);
		}

		vertexs[index][(1 * 6)] += -((vertexs[index][(1 * 6)] - goal[index][0]) / 100);
		vertexs[index][(1 * 6) + 1] += -((vertexs[index][(1 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(2 * 6)] += -((vertexs[index][(2 * 6)] - goal[index][2]) / 100);
		vertexs[index][(2 * 6) + 1] += -((vertexs[index][(2 * 6) + 1] - goal[index][3]) / 100);

		if (is_same_point(vertexs[index][(1 * 6)], vertexs[index][(1 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(2 * 6)], vertexs[index][(2 * 6) + 1], goal[index][2], goal[index][3]))
		{
			vertexs[index][(1 * 6)] = goal[index][0];
			vertexs[index][(1 * 6) + 1] = goal[index][1];

			vertexs[index][(2 * 6)] = goal[index][2];
			vertexs[index][(2 * 6) + 1] = goal[index][3];
			cur_shape[index]++;
		}
		break;
	case 1: // �ﰢ�� -> �簢��
		if (vertexs[index].size() < 36)
		{
			goal[index].clear();
			vertexs[index].push_back(vertexs[index][(1 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 5]);

			vertexs[index].push_back(vertexs[index][(1 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 5]);

			vertexs[index].push_back(vertexs[index][(2 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 5]);

			mid_x = (vertexs[index][(0 * 6)] + vertexs[index][(2 * 6)]) / 2;
			mid_y = (vertexs[index][(0 * 6) + 1] + vertexs[index][(1 * 6) + 1]) / 2;

			goal[index].push_back(mid_x - 0.25);
			goal[index].push_back(mid_y + 0.25f);

			goal[index].push_back(mid_x - 0.25);
			goal[index].push_back(mid_y + 0.25f);

			goal[index].push_back(mid_x + 0.25f);
			goal[index].push_back(mid_y + 0.25f);
		}

		vertexs[index][(1 * 6)] += -((vertexs[index][(1 * 6)] - goal[index][0]) / 100);
		vertexs[index][(1 * 6) + 1] += -((vertexs[index][(1 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(3 * 6)] += -((vertexs[index][(3 * 6)] - goal[index][2]) / 100);
		vertexs[index][(3 * 6) + 1] += -((vertexs[index][(3 * 6) + 1] - goal[index][3]) / 100);

		vertexs[index][(4 * 6)] += -((vertexs[index][(4 * 6)] - goal[index][4]) / 100);
		vertexs[index][(4 * 6) + 1] += -((vertexs[index][(4 * 6) + 1] - goal[index][5]) / 100);

		if (is_same_point(vertexs[index][(1 * 6)], vertexs[index][(1 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(3 * 6)], vertexs[index][(3 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(4 * 6)], vertexs[index][(4 * 6) + 1], goal[index][4], goal[index][5]))
		{
			vertexs[index][(1 * 6)] = goal[index][0];
			vertexs[index][(1 * 6) + 1] = goal[index][1];

			vertexs[index][(3 * 6)] = goal[index][2];
			vertexs[index][(3 * 6) + 1] = goal[index][3];

			vertexs[index][(4 * 6)] = goal[index][4];
			vertexs[index][(4 * 6) + 1] = goal[index][5];
			cur_shape[index]++;
		}
		break;
	case 2: // �簢�� -> ������
		if (vertexs[index].size() < 48)
		{
			goal[index].clear();
			vertexs[index].push_back(vertexs[index][(0 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(0 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(0 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(0 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(0 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(0 * 6) + 5]);

			vertexs[index].push_back(vertexs[index][(1 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(1 * 6) + 5]);

			vertexs[index].push_back(vertexs[index][(2 * 6) + 0]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 1]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 2]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 3]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 4]);
			vertexs[index].push_back(vertexs[index][(2 * 6) + 5]);

			mid_x = (vertexs[index][(0 * 6)] + vertexs[index][(2 * 6)]) / 2;
			mid_y = (vertexs[index][(0 * 6) + 1] + vertexs[index][(1 * 6) + 1]) / 2;

			goal[index].push_back(mid_x - 0.25f);
			goal[index].push_back(mid_y + 0.05f);//0

			goal[index].push_back(mid_x);
			goal[index].push_back(mid_y + 0.25f);//1

			goal[index].push_back(mid_x - 0.20f);
			goal[index].push_back(mid_y - 0.25);//2

			goal[index].push_back(mid_x);
			goal[index].push_back(mid_y + 0.25f);//3

			goal[index].push_back(mid_x + 0.25f);
			goal[index].push_back(mid_y + 0.05f);//4

			goal[index].push_back(mid_x + 0.20f);
			goal[index].push_back(mid_y - 0.25);//5

			goal[index].push_back(mid_x - 0.20f);
			goal[index].push_back(mid_y - 0.25);//6

			goal[index].push_back(mid_x);
			goal[index].push_back(mid_y + 0.25f);//7

			goal[index].push_back(mid_x + 0.20f);
			goal[index].push_back(mid_y - 0.25);//8

		}

		for (int i = 0; i < 9; i++)
		{
			vertexs[index][(i * 6)] += -((vertexs[index][(i * 6)] - goal[index][(i * 2)]) / 100);
			vertexs[index][(i * 6) + 1] += -((vertexs[index][(i * 6) + 1] - goal[index][(i * 2) + 1]) / 100);
		}

		if (is_same_point(vertexs[index][(0 * 6)], vertexs[index][(0 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(1 * 6)], vertexs[index][(1 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(2 * 6)], vertexs[index][(2 * 6) + 1], goal[index][4], goal[index][5])
			&& is_same_point(vertexs[index][(3 * 6)], vertexs[index][(3 * 6) + 1], goal[index][6], goal[index][7])
			&& is_same_point(vertexs[index][(4 * 6)], vertexs[index][(4 * 6) + 1], goal[index][8], goal[index][9])
			&& is_same_point(vertexs[index][(5 * 6)], vertexs[index][(5 * 6) + 1], goal[index][10], goal[index][11])
			&& is_same_point(vertexs[index][(6 * 6)], vertexs[index][(6 * 6) + 1], goal[index][12], goal[index][13])
			&& is_same_point(vertexs[index][(7 * 6)], vertexs[index][(7 * 6) + 1], goal[index][14], goal[index][15])
			&& is_same_point(vertexs[index][(8 * 6)], vertexs[index][(8 * 6) + 1], goal[index][16], goal[index][17]))
		{
			for (int i = 0; i < 9; i++)
			{
				vertexs[index][(i * 6)] = goal[index][(i * 2)];
				vertexs[index][(i * 6) + 1] = goal[index][(i * 2) + 1];
			}
			cur_shape[index]++;
		}
		break;
	case 3:// ������ -> �� ����
		goal[index].clear();
		mid_x = (vertexs[index][(0 * 6)] + vertexs[index][(4 * 6)]) / 2;
		mid_y = (vertexs[index][(1 * 6) + 1] + vertexs[index][(2 * 6) + 1]) / 2;

		goal[index].push_back(mid_x - 0.25f);
		goal[index].push_back(mid_y - 0.25f);//0

		goal[index].push_back(mid_x + 0.25f);
		goal[index].push_back(mid_y + 0.25f);//1

		cur_shape[index]++;
		break;
	case 4:// ������ -> �� �̵���
		vertexs[index][(0 * 6)] += -((vertexs[index][(0 * 6)] - goal[index][0]) / 100);
		vertexs[index][(0 * 6) + 1] += -((vertexs[index][(0 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(2 * 6)] += -((vertexs[index][(2 * 6)] - goal[index][0]) / 100);
		vertexs[index][(2 * 6) + 1] += -((vertexs[index][(2 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(6 * 6)] += -((vertexs[index][(6 * 6)] - goal[index][0]) / 100);
		vertexs[index][(6 * 6) + 1] += -((vertexs[index][(6 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(8 * 6)] += -((vertexs[index][(8 * 6)] - goal[index][0]) / 100);
		vertexs[index][(8 * 6) + 1] += -((vertexs[index][(8 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(5 * 6)] += -((vertexs[index][(5 * 6)] - goal[index][0]) / 100);
		vertexs[index][(5 * 6) + 1] += -((vertexs[index][(5 * 6) + 1] - goal[index][1]) / 100);

		vertexs[index][(1 * 6)] += -((vertexs[index][(1 * 6)] - goal[index][2]) / 100);
		vertexs[index][(1 * 6) + 1] += -((vertexs[index][(1 * 6) + 1] - goal[index][3]) / 100);

		vertexs[index][(3 * 6)] += -((vertexs[index][(3 * 6)] - goal[index][2]) / 100);
		vertexs[index][(3 * 6) + 1] += -((vertexs[index][(3 * 6) + 1] - goal[index][3]) / 100);

		vertexs[index][(7 * 6)] += -((vertexs[index][(7 * 6)] - goal[index][2]) / 100);
		vertexs[index][(7 * 6) + 1] += -((vertexs[index][(7 * 6) + 1] - goal[index][3]) / 100);

		vertexs[index][(4 * 6)] += -((vertexs[index][(4 * 6)] - goal[index][2]) / 100);
		vertexs[index][(4 * 6) + 1] += -((vertexs[index][(4 * 6) + 1] - goal[index][3]) / 100);

		if (is_same_point(vertexs[index][(0 * 6)], vertexs[index][(0 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(1 * 6)], vertexs[index][(1 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(2 * 6)], vertexs[index][(2 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(3 * 6)], vertexs[index][(3 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(4 * 6)], vertexs[index][(4 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(5 * 6)], vertexs[index][(5 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(6 * 6)], vertexs[index][(6 * 6) + 1], goal[index][0], goal[index][1])
			&& is_same_point(vertexs[index][(7 * 6)], vertexs[index][(7 * 6) + 1], goal[index][2], goal[index][3])
			&& is_same_point(vertexs[index][(8 * 6)], vertexs[index][(8 * 6) + 1], goal[index][0], goal[index][1]))
		{
			while (vertexs[index].size() != 12) {
				vertexs[index].pop_back();
			}
			cur_shape[index] = 0;
		}
		break;
	default:
		break;
	}
}


void TimerFunction(int value) {

	if (vertexs[4].size() == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			change_shape(i);
		}
	}
	else
	{
		change_shape(4);
	}
	glutTimerFunc(10, TimerFunction, 0);
	glutPostRedisplay();
}