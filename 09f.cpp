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
uniform_real_distribution<float> rand_speed(-0.01f, 0.01f);

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
void TimerFunction(int value);

//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

GLuint VAO, tri_VBO;

bool move_timer1 = false, move_timer2 = false, move_timer3 = false, move_timer4 = false;

vector<vector<float>> tri_vertex = {{},{},{},{}};
vector<vector<float>> speed = { {0,0,0},{0,0,0},{0,0,0},{0,0,0} };
vector<vector<float>> speed_last = { {0,0},{0,0},{0,0},{0,0} };
vector<vector<float>> move_3_max = { {1.0},{1.0},{1.0},{1.0} };
vector<vector<float>> move_4_seta = { {0},{0},{0},{0} };
vector<vector<float>> move_4_seta_speed = { {1},{1},{1},{1} };
vector<vector<float>> move_4_xy = { {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0} };


GLvoid InitBuffer()
{
	//--- VAO와 VBO 객체 생성
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &tri_VBO);
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

	if (!tri_vertex.empty()) {
		for (int i = 0; i < 4; i++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, tri_VBO);
			glBufferData(GL_ARRAY_BUFFER, tri_vertex[i].size() * sizeof(float), tri_vertex[i].data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // 위치
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // 색상
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glDrawArrays(GL_TRIANGLES, 0, tri_vertex[i].size() / 6);
		}
		// 각 점의 XYZ 좌표
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
	case '1':
		for (int i = 0; i < 4; i++)
		{
			speed[i][0] = rand_speed(eng);
			speed[i][1] = rand_speed(eng);
		}
		move_timer1 = !move_timer1;
		glutTimerFunc(10, TimerFunction, 1);
		break;
	case '2':
		for (int i = 0; i < 4; i++)
		{
			speed_last[i][0] = rand_speed(eng);
			speed_last[i][1] = rand_speed(eng);
			speed[i][0] = speed_last[i][0];
			speed[i][1] = 0;
		}
		move_timer2 = !move_timer2;
		glutTimerFunc(10, TimerFunction, 2);
		break;
	case '3':
		for (int i = 0; i < 4; i++)
		{
			speed_last[i][0] = rand_speed(eng);
			speed_last[i][1] = rand_speed(eng);
			speed[i][0] = speed_last[i][0];
			speed[i][1] = 0;
			speed[i][2] = 1.0f;
		}
		move_timer3 = !move_timer3;
		glutTimerFunc(10, TimerFunction, 3);
		break;
	case '4':
		for (int i = 0; i < 4; i++)
		{
			speed[i][0] = 0.001f;
			speed[i][1] = 0.1f;
			move_4_xy[i][0] = tri_vertex[i][0];
			move_4_xy[i][1] = tri_vertex[i][1];
			move_4_xy[i][2] = tri_vertex[i][6];
			move_4_xy[i][3] = tri_vertex[i][7];
			move_4_xy[i][4] = tri_vertex[i][12];
			move_4_xy[i][5] = tri_vertex[i][13];
		}
		move_timer4 = !move_timer4;
		glutTimerFunc(10, TimerFunction, 4);
		break;
	case 's':
		move_timer1 = false;
		move_timer2 = false;
		move_timer3 = false;
		move_timer4 = false;
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void Make_triangle(int index, float cx, float cy) {
	float ds = rand_size(eng);
	float tri_r = rand_color(eng);
	float tri_g = rand_color(eng);
	float tri_b = rand_color(eng);

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

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, width);
	GLclampf cy = convert_to_clampf_Y(y, height);
	if (state == GLUT_DOWN) {
		if (cx < 0 && cy >= 0) // 왼위
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(0, cx, cy);
			}
		}
		else if (cx >= 0 && cy >= 0) //오위
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(1, cx, cy);
			}
		}
		else if (cx < 0 && cy < 0) //왼아
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(2, cx, cy);
			}
		}
		else if (cx >= 0 && cy < 0) //오아
		{
			if (button == GLUT_LEFT_BUTTON)
			{
				Make_triangle(3, cx, cy);
			}
		}
	}
	glutPostRedisplay();
}

bool is_in_rect(float x, float y) {
	bool x_in = !(x < -1.0f || x > 1.0f);
	bool y_in = !(y < -1.0f || y > 1.0f);
	return (x_in && y_in);
}

void Zig_Zag(int index) {
	float mid_x, mid_y;

	tri_vertex[index][0] += speed[index][0];
	tri_vertex[index][1] += speed[index][1];

	tri_vertex[index][6] += speed[index][0];
	tri_vertex[index][7] += speed[index][1];

	tri_vertex[index][12] += speed[index][0];
	tri_vertex[index][13] += speed[index][1];

	if (speed[index][0] == 0)
	{
		speed[index][2]++;
	}

	mid_x = (tri_vertex[index][0] + tri_vertex[index][6] + tri_vertex[index][12]) / 3;
	mid_y = (tri_vertex[index][1] + tri_vertex[index][7] + tri_vertex[index][13]) / 3;

	if (mid_x < -1.0f || mid_x > 1.0f)
	{
		tri_vertex[index][0] -= speed[index][0];
		tri_vertex[index][1] -= speed[index][1];

		tri_vertex[index][6] -= speed[index][0];
		tri_vertex[index][7] -= speed[index][1];

		tri_vertex[index][12] -= speed[index][0];
		tri_vertex[index][13] -= speed[index][1];

		speed_last[index][0] = speed[index][0];

		speed[index][0] = 0;
		speed[index][1] = speed_last[index][1];
		speed[index][2] = 0;
	}
	else if (mid_y < -1.0f || mid_y > 1.0f)
	{
		speed[index][0] = 0;
		speed[index][1] = -speed[index][1];

		speed_last[index][1] = speed[index][1];
	}
	else if (speed[index][2] > 50)
	{
		tri_vertex[index][0] -= speed[index][0];
		tri_vertex[index][1] -= speed[index][1];

		tri_vertex[index][6] -= speed[index][0];
		tri_vertex[index][7] -= speed[index][1];

		tri_vertex[index][12] -= speed[index][0];
		tri_vertex[index][13] -= speed[index][1];

		speed_last[index][1] = speed[index][1];

		speed[index][0] = -speed_last[index][0];
		speed[index][1] = 0;
		speed[index][2] = 0;
	}
}

void move_rect(int index) {
	float mid_x, mid_y;

	tri_vertex[index][0] += speed[index][0];
	tri_vertex[index][1] += speed[index][1];

	tri_vertex[index][6] += speed[index][0];
	tri_vertex[index][7] += speed[index][1];

	tri_vertex[index][12] += speed[index][0];
	tri_vertex[index][13] += speed[index][1];

	mid_x = (tri_vertex[index][0] + tri_vertex[index][6] + tri_vertex[index][12]) / 3;
	mid_y = (tri_vertex[index][1] + tri_vertex[index][7] + tri_vertex[index][13]) / 3;

	if (mid_x < -1.0f || mid_x > 1.0f)
	{
		speed[index][0] = -speed[index][0];
	}
	else if (mid_y < -1.0f || mid_y > 1.0f)
	{
		speed[index][1] = -speed[index][1];
	}
}

void rect_spiral(int index) {
	float mid_x, mid_y;

	tri_vertex[index][0] += speed[index][0];
	tri_vertex[index][1] += speed[index][1];

	tri_vertex[index][6] += speed[index][0];
	tri_vertex[index][7] += speed[index][1];

	tri_vertex[index][12] += speed[index][0];
	tri_vertex[index][13] += speed[index][1];

	mid_x = (tri_vertex[index][0] + tri_vertex[index][6] + tri_vertex[index][12]) / 3;
	mid_y = (tri_vertex[index][1] + tri_vertex[index][7] + tri_vertex[index][13]) / 3;
	
	if (mid_x < -move_3_max[index][0] || mid_x > move_3_max[index][0])
	{
		if (speed[index][0] < 0)
		{
			tri_vertex[index][0] +=  0.11f;

			tri_vertex[index][6] +=  0.11f;

			tri_vertex[index][12] +=  0.11f;
		}
		else
		{
			tri_vertex[index][0] -=  0.11f;

			tri_vertex[index][6] -=  0.11f;

			tri_vertex[index][12] -=  0.11f;
		}

		speed_last[index][0] = speed[index][0];
		move_3_max[index][0] -= 0.05f;

		speed[index][0] = 0;
		speed[index][1] = -speed_last[index][1];

	}
	else if (mid_y < -move_3_max[index][0] || mid_y > move_3_max[index][0])
	{
		if (speed[index][1] < 0)
		{
			tri_vertex[index][1] += 0.11f;

			tri_vertex[index][7] += 0.11f;

			tri_vertex[index][13] += 0.11f;
		}
		else
		{
			tri_vertex[index][1] -=  0.11f;

			tri_vertex[index][7] -=  0.11f;

			tri_vertex[index][13] -=  0.11f;
		}

		speed_last[index][1] = speed[index][1];
		move_3_max[index][0] -= 0.05f;

		speed[index][0] = -speed_last[index][0];
		speed[index][1] = 0;
	}
}

void circle_spiral(int index) {
	float mid_x, mid_y;

	tri_vertex[index][0] = move_4_xy[index][0] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
	tri_vertex[index][1] = move_4_xy[index][1] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

	tri_vertex[index][6] = move_4_xy[index][2] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
	tri_vertex[index][7] = move_4_xy[index][3] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

	tri_vertex[index][12] = move_4_xy[index][4] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
	tri_vertex[index][13] = move_4_xy[index][5] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

	mid_x = (tri_vertex[index][0] + tri_vertex[index][6] + tri_vertex[index][12]) / 3;
	mid_y = (tri_vertex[index][1] + tri_vertex[index][7] + tri_vertex[index][13]) / 3;

	move_4_seta[index][0] += move_4_seta_speed[index][0];
	speed[index][1] += speed[index][0];

	if (move_4_seta[index][0] > 720)
	{
		move_4_seta_speed[index][0] = -move_4_seta_speed[index][0];
		speed[index][0] = -speed[index][0];

		tri_vertex[index][0] = move_4_xy[index][0] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
		tri_vertex[index][1] = move_4_xy[index][1] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

		tri_vertex[index][6] = move_4_xy[index][2] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
		tri_vertex[index][7] = move_4_xy[index][3] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

		tri_vertex[index][12] = move_4_xy[index][4] + (cos(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);
		tri_vertex[index][13] = move_4_xy[index][5] + (sin(move_4_seta[index][0] * 3.14 / 180) * speed[index][1]);

		move_4_seta[index][0] += move_4_seta_speed[index][0];
		speed[index][1] += speed[index][0];
	}
}

void TimerFunction(int value)
{
	switch (value)
	{
	case 1://튕기기
		for (int i = 0; i < 4; i++)
		{
			move_rect(i);
		}
		if (move_timer1) glutTimerFunc(10, TimerFunction, value);
		break;
	case 2://지그재그
		for (int i = 0; i < 4; i++)
		{
			Zig_Zag(i);
		}
		if (move_timer2) glutTimerFunc(10, TimerFunction, value);
		break;
	case 3://사각스파이럴
		for (int i = 0; i < 4; i++)
		{
			rect_spiral(i);
		}
		if (move_timer3) glutTimerFunc(10, TimerFunction, value);
		break;
	case 4://원스파이럴
		for (int i = 0; i < 4; i++)
		{
			circle_spiral(i);
		}
		if (move_timer4) glutTimerFunc(10, TimerFunction, value);
		break;
	default:
		break;
	}


	glutPostRedisplay();
}