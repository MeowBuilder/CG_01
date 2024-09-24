#include <iostream>
#include <random>

//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

using namespace std;

typedef struct BOX {
	int x1, y1, x2, y2;
	float r, g, b;
}Box;

Box four_box[4] = {};

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);

void Draw_Rect(Box box);
void init_Box() {
	random_device seeder;
	const auto seed = seeder.entropy() ? seeder() : time(nullptr);
	mt19937 eng(static_cast<mt19937::result_type>(seed));
	uniform_real_distribution<double> dist(0.0f, 1.0f);
	
	for (auto &box : four_box)
	{
		box.r = (GLclampf)dist(eng);
		box.g = (GLclampf)dist(eng);
		box.b = (GLclampf)dist(eng);
	}

	four_box[0].x1 = 0;
	four_box[0].y1 = 0;
	four_box[0].x2 = -400;
	four_box[0].y2 = 300;

	four_box[1].x1 = 0;
	four_box[1].y1 = 0;
	four_box[1].x2 = 400;
	four_box[1].y2 = 300;

	four_box[2].x1 = 0;
	four_box[2].y1 = 0;
	four_box[2].x2 = -400;
	four_box[2].y2 = -300;

	four_box[3].x1 = 0;
	four_box[3].y1 = 0;
	four_box[3].x2 = 400;
	four_box[3].y2 = -300;
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{ //--- 윈도우 생성하기
	glutInit(&argc, argv); // glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 디스플레이 모드 설정
	glutInitWindowPosition(0, 0); // 윈도우의 위치 지정
	glutInitWindowSize(800, 600); // 윈도우의 크기 지정
	glutCreateWindow("Example1"); // 윈도우 생성(윈도우 이름)

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew 초기화
	{
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else std::cout << "GLEW Initialized\n";
	init_Box();
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutMouseFunc(Mouse); //키보드 입력 콜백 함수
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	for (const auto& box : four_box)
	{
		Draw_Rect(box);
	}

	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (x < 400 && y < 300) // 왼위
		{

		}
		else if (x >= 400 && y < 300 ) //오위
		{

		}
		else if (x < 400 && y >= 300) //왼아
		{

		}
		else if (x >= 400 && y >= 300) //오아
		{

		}
	}
		
}

void Draw_Rect(Box box) {
	glColor3f(box.r, box.g, box.b);
	glRectf(box.x1, box.y1, box.x2, box.y2);
}