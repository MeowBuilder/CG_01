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
uniform_real_distribution<float> rand_xy(-1, 1);

//--- 필요한 헤더파일 선언
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

//--- 사용자 정의 함수
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid Mouse(int button, int state, int x, int y);
void TimerFunction(int value);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

GLuint VAO, tri_VBO;

bool is_p = true;
int spiral_make_count = 1;

vector<float> Start_vertex = { };
vector<vector<float>> Spiral_vertex = { };
vector<float> Spiral_radius = { };
vector<float> spiral_seta = { };
vector<float> spiral_seta_speed = { };

GLfloat rColor, gColor, bColor;

GLvoid InitBuffer()
{
	//--- VAO와 VBO 객체 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &tri_VBO);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	width = 800;
	height = 600;
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(width, height);
	glutCreateWindow("Example1");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- VAO/VBO 버퍼 초기화
	shaderProgramID = make_shaderProgram();
	InitBuffer();

	glutTimerFunc(10, TimerFunction, 0);

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
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
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

GLuint make_shaderProgram()
{
	make_vertexShaders();
	make_fragmentShaders();
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기

	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기

	return shaderID;
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	if (!Spiral_vertex.empty()) {
		for (int i = 0; i < (Start_vertex.size() / 5); i++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
			glBufferData(GL_ARRAY_BUFFER, Spiral_vertex[i].size() * sizeof(float), Spiral_vertex[i].data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			if (is_p)
			{
				glDrawArrays(GL_POINTS, 0, Spiral_vertex[i].size() / 6);
			}
			else
			{
				glDrawArrays(GL_LINE_STRIP, 0, Spiral_vertex[i].size() / 6);
			}
		}
	}


	glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid Keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'p':
		is_p = true;
		break;
	case 'l':
		is_p = false;
		break;
	case '1':
		spiral_make_count = 1;
		break;
	case '2':
		spiral_make_count = 2;
		break;
	case '3':
		spiral_make_count = 3;
		break;
	case '4':
		spiral_make_count = 4;
		break;
	case '5':
		spiral_make_count = 5;
		break;
	case 'c':
		Start_vertex.clear();
		Spiral_vertex.clear();
		Spiral_radius.clear();
		spiral_seta.clear();
		spiral_seta_speed.clear();
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void setBG_Color() {
	rColor = rand_color(eng);
	gColor = rand_color(eng);
	bColor = rand_color(eng);
}

void Mouse(int button, int state, int x, int y)
{
	vector<float> newpoint = {};
	GLclampf cx = convert_to_clampf_X(x, width);
	GLclampf cy = convert_to_clampf_Y(y, height);
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		setBG_Color();
		for (int i = 0; i < spiral_make_count; i++)
		{
			if (i > 0)
			{
				Start_vertex.push_back(rand_xy(eng));
				Start_vertex.push_back(rand_xy(eng));
				Start_vertex.push_back(rand_color(eng));
				Start_vertex.push_back(rand_color(eng));
				Start_vertex.push_back(rand_color(eng));
				Spiral_radius.push_back(0.0f);
				spiral_seta.push_back(0.0f);
				spiral_seta_speed.push_back(5.0f);
				spiral_seta_speed.push_back(0.001f);
				Spiral_vertex.push_back(newpoint);
			}
			else
			{
				Start_vertex.push_back(cx);
				Start_vertex.push_back(cy);
				Start_vertex.push_back(rand_color(eng));
				Start_vertex.push_back(rand_color(eng));
				Start_vertex.push_back(rand_color(eng));
				Spiral_radius.push_back(0.0f);
				spiral_seta.push_back(0.0f);
				spiral_seta_speed.push_back(5.0f);
				spiral_seta_speed.push_back(0.001f);
				Spiral_vertex.push_back(newpoint);
			}
		}
	}
	glutPostRedisplay();
}

void circle_spiral(int index) {
	if (spiral_seta_speed[index] < 0 && spiral_seta[index] < -180) return;


	Spiral_vertex[index].push_back(Start_vertex[(index * 5)] + (cos(spiral_seta[index] * 3.14 / 180) * Spiral_radius[index]));
	Spiral_vertex[index].push_back(Start_vertex[(index * 5) + 1] + (sin(spiral_seta[index] * 3.14 / 180) * Spiral_radius[index]));
	Spiral_vertex[index].push_back(0.0f);
	Spiral_vertex[index].push_back(Start_vertex[(index * 5) + 2]);
	Spiral_vertex[index].push_back(Start_vertex[(index * 5) + 3]);
	Spiral_vertex[index].push_back(Start_vertex[(index * 5) + 4]);

	spiral_seta[index] += spiral_seta_speed[(index * 2)];
	Spiral_radius[index] += spiral_seta_speed[(index * 2) + 1];
	
	if (spiral_seta[index] > 720)
	{
		spiral_seta_speed[(index * 2)] = -spiral_seta_speed[(index * 2)];
		spiral_seta_speed[(index * 2) + 1] = -spiral_seta_speed[(index * 2) + 1];

		spiral_seta[index] -= 180;

		Start_vertex[(index * 5)] += (Spiral_radius[index] * 2);

	}
}

void TimerFunction(int value)
{
	for (int i = 0; i < (Start_vertex.size() / 5); i++)
	{
		circle_spiral(i);
	}
	glutTimerFunc(10, TimerFunction, 0);
	glutPostRedisplay();
}