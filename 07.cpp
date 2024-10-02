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
uniform_int_distribution<int> sel_shape(0, 3);
uniform_real_distribution<float> rend_color(0, 1);

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
void move_shape(int selected, float dx, float dy);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

GLuint VAO, point_VBO, line_VBO, tri_VBO, rect_VBO;

char cur_state = NULL;

vector<float> point_vertex;
vector<float> line_vertex;
vector<float> tri_vertex;
vector<float> rect_vertex;

int rect_count = 0;
int shape_count = 0;
int selected_shape = 0;
int selected = 0;


GLvoid InitBuffer()
{
	//--- VAO와 VBO 객체 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &point_VBO);
	glGenBuffers(1, &line_VBO);
	glGenBuffers(1, &tri_VBO);
	glGenBuffers(1, &rect_VBO);
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
	if(!result)
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_LINES, 0, line_vertex.size() / 6);
		// 각 점의 XYZ 좌표
	}


	if (!tri_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
		glBufferData(GL_ARRAY_BUFFER, tri_vertex.size() * sizeof(float), tri_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glDrawArrays(GL_TRIANGLES, 0, tri_vertex.size() / 6);
		// 각 점의 XYZ 좌표
	}


	if (!rect_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, rect_VBO);
		glBufferData(GL_ARRAY_BUFFER, rect_vertex.size() * sizeof(float), rect_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		int Draw_vertex = (rect_vertex.size() / 6) / 6;

		glDrawArrays(GL_TRIANGLES, 0, 6 * Draw_vertex);
		// 각 점의 XYZ 좌표
	}

	if (!point_vertex.empty()) {
		glBindBuffer(GL_ARRAY_BUFFER, point_VBO);
		glBufferData(GL_ARRAY_BUFFER, point_vertex.size() * sizeof(float), point_vertex.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, point_vertex.size() / 6); // 각 점의 XYZ 좌표
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
	case 'l':
	case 't':
	case 'r':
		cur_state = key;
		break;
	case 'w':
		selected_shape = sel_shape(eng);
		move_shape(selected_shape, 0, 0.5f);
		break;
	case 'a':
		selected_shape = sel_shape(eng);
		move_shape(selected_shape, - 0.5f, 0);
		break;
	case 's':
		selected_shape = sel_shape(eng);
		move_shape(selected_shape, 0, -0.5f);
		break;
	case 'd':
		selected_shape = sel_shape(eng);
		move_shape(selected_shape, 0.5f, 0);
		break;
	case 'c':
		point_vertex.clear();
		line_vertex.clear();
		tri_vertex.clear();
		rect_vertex.clear();

		rect_count = 0;
		shape_count = 0;
		selected_shape = 0;
		selected = 0;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

bool is_in_rect(float x,float y) {
	bool x_in = !(x < -1.0f || x > 1.0f);
	bool y_in = !(y < -1.0f || y > 1.0f);
	return (x_in && y_in);
}

void move_shape(int selected, float dx,float dy) {
	bool bad_for_start = true;
	while (bad_for_start)
	{
		switch (selected)
		{
		case 0://점
			if ((point_vertex.size() / 6) > 0) bad_for_start = false;
			break;
		case 1://선
			if (((line_vertex.size() / 6) / 2) > 0) bad_for_start = false;
			break;
		case 2://삼각형
			if (((tri_vertex.size() / 6) / 3) > 0) bad_for_start = false;
			break;
		case 3://사각형
			if (((rect_vertex.size() / 6) / 6) > 0) bad_for_start = false;
			break;
		}
		if (!bad_for_start) break;
		selected = sel_shape(eng);
	}


	uniform_int_distribution<int> random(0, 100);
	int ver_sel = 0;
	int mid_x, mid_y;
	switch (selected)
	{
	case 0://점
		ver_sel = random(eng) % (point_vertex.size() / 6);
		mid_x = point_vertex[((ver_sel + 0) * 6)];
		mid_y = point_vertex[((ver_sel + 0) * 6) + 1];

		if (!is_in_rect(mid_x + dx, mid_y + dy)) break;

		point_vertex[((ver_sel + 0) * 6)] += dx;
		point_vertex[((ver_sel + 0) * 6) + 1] += dy;

		break;
	case 1://선
		ver_sel = random(eng) % ((line_vertex.size() / 6) / 2);
		mid_x = (line_vertex[((ver_sel + 0) * 6)] + line_vertex[((ver_sel + 1) * 6)]) / 2;
		mid_y = (line_vertex[((ver_sel + 0) * 6) + 1] + line_vertex[((ver_sel + 1) * 6) + 1]) / 2;

		if (!is_in_rect(mid_x + dx, mid_y + dy)) break;

		line_vertex[((ver_sel + 0) * 6)] += dx;
		line_vertex[((ver_sel + 0) * 6) + 1] += dy;
		line_vertex[((ver_sel + 1) * 6)] += dx;
		line_vertex[((ver_sel + 1) * 6) + 1] += dy;
		break;
	case 2://삼각형
		ver_sel = random(eng) % ((tri_vertex.size() / 6) / 3);

		mid_x = (tri_vertex[((ver_sel + 0) * 6)] + tri_vertex[((ver_sel + 1) * 6)] + tri_vertex[((ver_sel + 2) * 6)]) / 3;
		mid_y = (tri_vertex[((ver_sel + 0) * 6) + 1] + tri_vertex[((ver_sel + 1) * 6) + 1] + tri_vertex[((ver_sel + 2) * 6) + 1]) / 3;

		if (!is_in_rect(mid_x + dx, mid_y + dy)) break;

		tri_vertex[((ver_sel + 0) * 6)] += dx;
		tri_vertex[((ver_sel + 0) * 6) + 1] += dy;
		tri_vertex[((ver_sel + 1) * 6)] += dx;
		tri_vertex[((ver_sel + 1) * 6) + 1] += dy;
		tri_vertex[((ver_sel + 2) * 6)] += dx;
		tri_vertex[((ver_sel + 2) * 6) + 1] += dy;
		break;
	case 3://사각형
		ver_sel = random(eng) % ((rect_vertex.size() / 6) / 6);

		mid_x = (rect_vertex[((ver_sel + 0) * 6)] + rect_vertex[((ver_sel + 1) * 6)] + rect_vertex[((ver_sel + 2) * 6)] + rect_vertex[((ver_sel + 3) * 6)]) / 4;
		mid_y = (rect_vertex[((ver_sel + 0) * 6) + 1] + rect_vertex[((ver_sel + 1) * 6) + 1] + rect_vertex[((ver_sel + 2) * 6) + 1] + rect_vertex[((ver_sel + 3) * 6) + 1]) / 4;

		if (!is_in_rect(mid_x + dx, mid_y + dy)) break;

		rect_vertex[((ver_sel + 0) * 6)] += dx;
		rect_vertex[((ver_sel + 0) * 6) + 1] += dy;
		rect_vertex[((ver_sel + 1) * 6)] += dx;
		rect_vertex[((ver_sel + 1) * 6) + 1] += dy;
		rect_vertex[((ver_sel + 2) * 6)] += dx;
		rect_vertex[((ver_sel + 2) * 6) + 1] += dy;
		rect_vertex[((ver_sel + 3) * 6)] += dx;
		rect_vertex[((ver_sel + 3) * 6) + 1] += dy;

		rect_vertex[((ver_sel + 4) * 6)] += dx;
		rect_vertex[((ver_sel + 4) * 6) + 1] += dy;
		rect_vertex[((ver_sel + 5) * 6)] += dx;
		rect_vertex[((ver_sel + 5) * 6) + 1] += dy;
		break;
	}
}

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, width);
	GLclampf cy = convert_to_clampf_Y(y, height);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		switch (cur_state)
		{
		case 'p':
			if (shape_count > 9) break;
			point_vertex.push_back(cx);
			point_vertex.push_back(cy);
			point_vertex.push_back(0.0f);
			point_vertex.push_back(rend_color(eng));
			point_vertex.push_back(rend_color(eng));
			point_vertex.push_back(rend_color(eng));
			shape_count++;
			break;
		case 'l':
			if (shape_count > 9) break;
			line_vertex.push_back(cx);
			line_vertex.push_back(cy);
			line_vertex.push_back(0.0f);
			line_vertex.push_back(rend_color(eng));
			line_vertex.push_back(rend_color(eng));
			line_vertex.push_back(rend_color(eng));
			if ((line_vertex.size() / 6)%2 == 0) shape_count++;
			break;
		case 't':
			if (shape_count > 9) break;
			tri_vertex.push_back(cx);
			tri_vertex.push_back(cy);
			tri_vertex.push_back(0.0f);
			tri_vertex.push_back(rend_color(eng));
			tri_vertex.push_back(rend_color(eng));
			tri_vertex.push_back(rend_color(eng));
			if ((tri_vertex.size() / 6) % 3 == 0) shape_count++;
			break;
		case 'r':
			if (shape_count > 9) break;
			rect_vertex.push_back(cx);
			rect_vertex.push_back(cy);
			rect_vertex.push_back(0.0f);
			rect_vertex.push_back(rend_color(eng));
			rect_vertex.push_back(rend_color(eng));
			rect_vertex.push_back(rend_color(eng));
			rect_count++;
			if (rect_count == 4)
			{
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 24]);

				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);//이전 버텍스의 마지막
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);
				rect_vertex.push_back(rect_vertex[rect_vertex.size() - 18]);

				rect_count = 0;
				shape_count++;
			}
			break;
		}
	}
	glutPostRedisplay();
}