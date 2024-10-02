#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

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


//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

GLuint VAO, point_VBO, line_VBO, tri_VBO, rect_VBO;

char cur_state = NULL;

vector<float> point_vertex;
vector<float> line_vertex;
vector<float> tri_vertex;
vector<float> rect_vertex;

GLvoid InitBuffer()
{
	//--- VAO�� VBO ��ü ����
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &point_VBO);
	glGenBuffers(1, &line_VBO);
	glGenBuffers(1, &tri_VBO);
	glGenBuffers(1, &rect_VBO);
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
	if(!result)
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
	rColor = 1.0;
	gColor = 1.0;
	bColor = 1.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	if (!line_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
		glBufferData(GL_ARRAY_BUFFER, line_vertex.size() * sizeof(float), line_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_LINES, 0, line_vertex.size() / 6);
		// �� ���� XYZ ��ǥ
	}


	if (!tri_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
		glBufferData(GL_ARRAY_BUFFER, tri_vertex.size() * sizeof(float), tri_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, tri_vertex.size() / 6);
		// �� ���� XYZ ��ǥ
	}


	if (!rect_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, rect_VBO);
		glBufferData(GL_ARRAY_BUFFER, rect_vertex.size() * sizeof(float), rect_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		for (int i = 0; i <= rect_vertex.size()/6; i++)
		{
			if (i % 4 == 0)
			{
				glDrawArrays(GL_TRIANGLE_STRIP, i, i + 4);//�����̻���
			}
		}
		// �� ���� XYZ ��ǥ
	}

	if (!point_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, point_VBO);
		glBufferData(GL_ARRAY_BUFFER, point_vertex.size() * sizeof(float), point_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // ��ġ
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // ����
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, point_vertex.size() / 6); // �� ���� XYZ ��ǥ
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
	case 'p':
	case 'l':
	case 't':
	case 'r':
		cur_state = key;
		break;
	case 'w':

		break;
	case 'a':
		cout << point_vertex.size();
		break;
	case 's':

		break;
	case 'd':

		break;
	case 'c':

		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, width);
	GLclampf cy = convert_to_clampf_Y(y, height);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		switch (cur_state)
		{
		case 'p':
			point_vertex.push_back(cx);
			point_vertex.push_back(cy);
			point_vertex.push_back(0.0f);
			point_vertex.push_back(1.0f);
			point_vertex.push_back(0.0f);
			point_vertex.push_back(0.0f);
			break;
		case 'l':
			line_vertex.push_back(cx);
			line_vertex.push_back(cy);
			line_vertex.push_back(0.0f);
			line_vertex.push_back(1.0f);
			line_vertex.push_back(0.0f);
			line_vertex.push_back(0.0f);
			break;
		case 't':
			tri_vertex.push_back(cx);
			tri_vertex.push_back(cy);
			tri_vertex.push_back(0.0f);
			tri_vertex.push_back(1.0f);
			tri_vertex.push_back(0.0f);
			tri_vertex.push_back(0.0f);
			break;
		case 'r':
			rect_vertex.push_back(cx);
			rect_vertex.push_back(cy);
			rect_vertex.push_back(0.0f);
			rect_vertex.push_back(1.0f);
			rect_vertex.push_back(0.0f);
			rect_vertex.push_back(0.0f);
			break;
		}
	}
	glutPostRedisplay();
}