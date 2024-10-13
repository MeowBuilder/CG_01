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
uniform_real_distribution<float> rand_xy(-1, 1);
uniform_real_distribution<float> rand_speed(-0.01f, 0.01f);
uniform_int_distribution<int> rand_shape(1, 5);

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
void TimerFunction(int value);
void change_shape(int index);
vector<float> make_random_shape();
void Mouse(int button, int state, int x, int y);
void DragMouse(int x, int y);
void make_vertex(vector<float>* vec, float attribute[6]);
void move_rect(int index);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체
GLuint VAO, VBO, Line_VBO;
int cur_selected = -1;

vector<vector<float>> vertexs = {};
vector<int> cur_shape = {};
vector<vector<float>> speed = {};

vector<vector<float>> goal = {};

GLvoid InitBuffer()
{
	//--- VAO와 VBO 객체 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &Line_VBO);
}

//--- 메인 함수
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
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(DragMouse);
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
	GLfloat rColor, gColor, bColor;
	vector<float> triangle;
	rColor = 1.0;
	gColor = 1.0;
	bColor = 1.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	if (!vertexs.empty()) {
		for (int i = 0; i < vertexs.size(); i++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertexs[i].size() * sizeof(float), vertexs[i].data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			if (cur_shape[i] == 1)
			{
				glPointSize(5.0);
				glDrawArrays(GL_POINTS, 0, vertexs[i].size() / 6);
			}
			else if (cur_shape[i] == 2)
			{
				glDrawArrays(GL_LINES, 0, vertexs[i].size() / 6);
			}
			else
			{
				glDrawArrays(GL_TRIANGLES, 0, vertexs[i].size() / 6);
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
	case 'r':
		for (int i = 0; i < 30; i++)
		{
			vertexs.push_back(make_random_shape());
		}
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

bool is_in_shape(vector<float> shape, float cx, float cy) {
	float midx = 0, midy = 0;
	for (int i = 0; i < shape.size()/6; i++)
	{
		midx += shape[i * 6];
		midy += shape[(i * 6) + 1];
	}
	midx = midx / (shape.size() / 6);
	midy = midy / (shape.size() / 6);
	return sqrt((midx - cx) * (midx - cx) + (midy - cy) * (midy - cy)) < 0.05f;
}

bool is_on_other(vector<float> shape, vector<float> other) {
	float midx = 0, midy = 0;
	float midx_ot = 0, midy_ot = 0;
	for (int i = 0; i < shape.size() / 6; i++)
	{
		midx += shape[i * 6];
		midy += shape[(i * 6) + 1];
	}
	midx = midx / (shape.size() / 6);
	midy = midy / (shape.size() / 6);

	for (int i = 0; i < other.size() / 6; i++)
	{
		midx_ot += other[i * 6];
		midy_ot += other[(i * 6) + 1];
	}
	midx_ot = midx_ot / (other.size() / 6);
	midy_ot = midy_ot / (other.size() / 6);
	return sqrt((midx - midx_ot) * (midx - midx_ot) + (midy - midy_ot) * (midy - midy_ot)) < 0.1f;
}

void merge(int index_this, int index_other) {
	vector<float> newshape;
	float x_t = 0, y_t = 0, r_t = 0, g_t = 0, b_t = 0;
	float x_o = 0, y_o = 0, r_o = 0, g_o = 0, b_o = 0;
	cur_shape[index_this] = cur_shape[index_this] + cur_shape[index_other];
	if (cur_shape[index_this] > 5)
	{
		cur_shape[index_this] -= 5;
	}
	for (int i = 0; i < vertexs[index_this].size()/6; i++)
	{
		x_t += vertexs[index_this][(i * 6) + 0];
		y_t += vertexs[index_this][(i * 6) + 1];
		r_t += vertexs[index_this][(i * 6) + 3];
		g_t += vertexs[index_this][(i * 6) + 4];
		b_t += vertexs[index_this][(i * 6) + 5];
	}
	x_t /= vertexs[index_this].size() / 6;
	y_t /= vertexs[index_this].size() / 6;
	r_t /= vertexs[index_this].size() / 6;
	g_t /= vertexs[index_this].size() / 6;
	b_t /= vertexs[index_this].size() / 6;
	for (int i = 0; i < vertexs[index_other].size() / 6; i++)
	{
		x_o += vertexs[index_other][(i * 6) + 0];
		y_o += vertexs[index_other][(i * 6) + 1];
		r_o += vertexs[index_other][(i * 6) + 3];
		g_o += vertexs[index_other][(i * 6) + 4];
		b_o += vertexs[index_other][(i * 6) + 5];
	}
	x_o /= vertexs[index_other].size() / 6;
	y_o /= vertexs[index_other].size() / 6;
	r_o /= vertexs[index_other].size() / 6;
	g_o /= vertexs[index_other].size() / 6;
	b_o /= vertexs[index_other].size() / 6;
	float x = (x_t + x_o / 2);
	float y = (y_t + y_o / 2);

	float attrib[6] = { x, y, 0.0, (r_t + r_o / 2), (g_t + g_o / 2), (b_t + b_o / 2) };
	vertexs[index_this].clear();
	switch (cur_shape[index_this])
	{
	case 1:
		make_vertex(&newshape, attrib);
		vertexs[index_this] = newshape;
		break;
	case 2:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index_this] = newshape;
		break;
	case 3:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index_this] = newshape;
		break;
	case 4:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index_this] = newshape;
		break;
	case 5:
		attrib[0] = x - 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index_this] = newshape;
		break;
	default:
		break;
	}

	speed[index_this].push_back(rand_speed(eng));
	speed[index_this].push_back(rand_speed(eng));

	vertexs.erase(vertexs.begin() + index_other);
	goal.erase(goal.begin() + index_other);
	cur_shape.erase(cur_shape.begin() + index_other);
	speed.erase(speed.begin() + index_other);
}

void Mouse(int button, int state, int x, int y)
{
	bool loop_break = false;
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
		{
			for (int i = 0; i < vertexs.size(); i++)
			{
				if (is_in_shape(vertexs[i],cx,cy))
				{
					cur_selected = i;
					break;
				}
			}
		}
		else if (state == GLUT_UP)
		{
			for (int i = 0; i < vertexs.size(); i++) {
				if (i == cur_selected)
				{
					for (int j = 0; j < vertexs.size(); j++) {
						if (i != j && is_on_other(vertexs[i],vertexs[j]))
						{
							merge(i, j); //야랄남
							loop_break = true;
							break;
						}
					}
					if (loop_break) break;
				}
			}
			loop_break = false;
			cur_selected = -1;
		}
	}

	glutPostRedisplay();
}

void move_shape(int index, float cx, float cy) {
	vector<float> newshape;
	float x_t = 0, y_t = 0, r_t = 0, g_t = 0, b_t = 0;
	for (int i = 0; i < vertexs[index].size() / 6; i++)
	{
		x_t += vertexs[index][(i * 6) + 0];
		y_t += vertexs[index][(i * 6) + 1];
		r_t += vertexs[index][(i * 6) + 3];
		g_t += vertexs[index][(i * 6) + 4];
		b_t += vertexs[index][(i * 6) + 5];
	}
	x_t /= vertexs[index].size() / 6;
	y_t /= vertexs[index].size() / 6;
	r_t /= vertexs[index].size() / 6;
	g_t /= vertexs[index].size() / 6;
	b_t /= vertexs[index].size() / 6;

	

	float attrib[6] = { cx, cy, 0.0, r_t, g_t, b_t };
	vertexs[index].clear();
	switch (cur_shape[index])
	{
	case 1:
		make_vertex(&newshape, attrib);
		vertexs[index] = newshape;
		cur_shape[index] = cur_shape[index];
		break;
	case 2:
		attrib[0] = cx - 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx + 0.2;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index] = newshape;
		cur_shape[index] = cur_shape[index];
		break;
	case 3:
		attrib[0] = cx - 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx + 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index] = newshape;
		cur_shape[index] = cur_shape[index];
		break;
	case 4:
		attrib[0] = cx - 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx - 0.2;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx + 0.2;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx + 0.2;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx + 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx - 0.2;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index] = newshape;
		cur_shape[index] = cur_shape[index];
		break;
	case 5:
		attrib[0] = cx - 0.15;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx - 0.2;
		attrib[1] = cy + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = cx;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx + 0.2;
		attrib[1] = cy + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = cx + 0.15;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = cx;
		attrib[1] = cy + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx - 0.15;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = cx + 0.15;
		attrib[1] = cy - 0.2;
		make_vertex(&newshape, attrib);

		vertexs[index] = newshape;
		cur_shape[index] = cur_shape[index];
		break;
	default:
		break;
	}
}

void DragMouse(int x, int y) {
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);

	for (int i = 0; i < vertexs.size(); i++) {
		if (i == cur_selected)
		{
			move_shape(i, cx, cy);
		}
	}
	glutPostRedisplay();
}

bool is_same_point(float this_x, float this_y, float goal_x, float goal_y) {
	return sqrt((this_x - goal_x) * (this_x - goal_x) + (this_y - goal_y) * (this_y - goal_y)) < 0.05f;
}

void change_shape(int index) {
	float mid_x, mid_y;

	switch (cur_shape[index])
	{
	case 1: // 선 -> 삼각형
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
	case 2: // 삼각형 -> 사각형
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
	case 3: // 사각형 -> 오각형
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
	case 4:// 오각형 -> 선 최초
		goal[index].clear();
		mid_x = (vertexs[index][(0 * 6)] + vertexs[index][(4 * 6)]) / 2;
		mid_y = (vertexs[index][(1 * 6) + 1] + vertexs[index][(2 * 6) + 1]) / 2;

		goal[index].push_back(mid_x - 0.25f);
		goal[index].push_back(mid_y - 0.25f);//0

		goal[index].push_back(mid_x + 0.25f);
		goal[index].push_back(mid_y + 0.25f);//1

		cur_shape[index]++;
		break;
	case 5:// 오각형 -> 선 이동중
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
			cur_shape[index] = 1;
		}
		break;
	default:
		break;
	}
}

void TimerFunction(int value) {

	for (int i = 0; i < vertexs.size(); i++)
	{
		if (speed[i].size() > 0)
		{
			if (speed[i][0] != 0)
			{
				move_rect(i);

			}
		}
	}
	glutTimerFunc(10, TimerFunction, 0);
	glutPostRedisplay();
}

void make_vertex(vector<float>* vec,float attribute[6]) {
	for (int i = 0; i < 6; i++)
	{
		vec->push_back(attribute[i]);
	}
}

vector<float> make_random_shape() {
	vector<float> newshape;
	goal.push_back(newshape);
	speed.push_back(newshape);
	int num_of_vertex = rand_shape(eng);
	float x = rand_xy(eng);
	float y = rand_xy(eng);
	float r = rand_color(eng);
	float g = rand_color(eng);
	float b = rand_color(eng);

	float attrib[6] = { x, y, 0.0, r, g, b };

	switch (num_of_vertex)
	{
	case 1:
		make_vertex(&newshape, attrib);
		cur_shape.push_back(num_of_vertex);
		break;
	case 2:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		cur_shape.push_back(num_of_vertex);
		break;
	case 3:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		cur_shape.push_back(num_of_vertex);
		break;
	case 4:
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x + 0.2;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		cur_shape.push_back(num_of_vertex);
		break;
	case 5:
		attrib[0] = x - 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.2;
		attrib[1] = y + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.2;
		attrib[1] = y + 0.1;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);

		attrib[0] = x;
		attrib[1] = y + 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x - 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		attrib[0] = x + 0.15;
		attrib[1] = y - 0.2;
		make_vertex(&newshape, attrib);
		cur_shape.push_back(num_of_vertex);
		break;
	default:
		break;
	}
	return newshape;
}

void move_rect(int index) {
	float mid_x = 0, mid_y = 0;

	for (int i = 0; i < vertexs[index].size() / 6; i++)
	{
		vertexs[index][(i * 6) + 0] += speed[index][0];
		vertexs[index][(i * 6) + 1] += speed[index][1];
	}

	for (int i = 0; i < vertexs[index].size() / 6; i++)
	{
		mid_x += vertexs[index][(i * 6) + 0];
		mid_y += vertexs[index][(i * 6) + 1];
	}

	mid_x = mid_x / (vertexs[index].size() / 6);
	mid_y = mid_y / (vertexs[index].size() / 6);

	if (mid_x < -1.0f || mid_x > 1.0f)
	{
		speed[index][0] = -speed[index][0];
	}
	else if (mid_y < -1.0f || mid_y > 1.0f)
	{
		speed[index][1] = -speed[index][1];
	}
}