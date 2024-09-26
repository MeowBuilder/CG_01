#include <iostream>
#include <random>

//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

using namespace std;

class Box
{
public:
	Box() {
		random_device seeder;
		const auto seed = seeder.entropy() ? seeder() : time(nullptr);
		mt19937 eng(static_cast<mt19937::result_type>(seed));
		uniform_real_distribution<double> dist(0.0f, 1.0f);

		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);

		this->x1 = 0.0f;
		this->y1 = 0.0f;
		this->x2 = 0.0f;
		this->y2 = 0.0f;
	}

	void Draw_Box() {
		glColor3f(this->r, this->g, this->b);
		glRectf(this->x1, this->y1, this->x2, this->y2);
	}

	void set_rect(float x1, float y1, float x2, float y2) {
		this->x1 = x1;
		this->y1 = y1;
		this->x2 = x2;
		this->y2 = y2;
	}
private:
	GLclampf r, g, b;
	GLclampf x1, y1, x2, y2;
};

Box four_box[4];

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);

GLclampf convert_to_clampf_X(int x, int w) {
	return (x - (w / 2.0f)) / (w / 2.0f);
}
GLclampf convert_to_clampf_Y(int y, int h) {
	return (y - (h / 2.0f)) / (-h / 2.0f);
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

	int n = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			four_box[n].set_rect(j - 1, -i, j, (1 - i)); //박스 위치 초기화
			n++;
		}
	}

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutMouseFunc(Mouse); //키보드 입력 콜백 함수
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // 설정된 색으로 전체를 칠하기
	// 그리기 부분 구현: 그리기 관련 부분이 여기에 포함된다.

	for (int i = 0; i < 4; i++)
	{
		four_box[i].Draw_Box();
	}

	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (cx < 0 && cy >= 0) // 왼위
		{
			cout << "왼위" << endl;
		}
		else if (cx >= 0 && cy >= 0) //오위
		{
			cout << "오위" << endl;
		}
		else if (cx < 0 && cy < 0) //왼아
		{
			cout << "왼아" << endl;
		}
		else if (cx >= 0 && cy < 0) //오아
		{
			cout << "오아" << endl;
		}
	}
		
}