#include <iostream>
#include <random>
#include <vector>

//--- 필요한 헤더파일 include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

using namespace std;

random_device seeder;
const auto seed = seeder.entropy() ? seeder() : time(nullptr);
mt19937 eng(static_cast<mt19937::result_type>(seed));
uniform_real_distribution<double> dist(0.0f, 1.0f);
uniform_real_distribution<double> xy(-1.0f, 1.0f);

class Box
{
public:
	bool clicked;
	Box() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);

		this->x1 = 0.0f;
		this->y1 = 0.0f;
		this->x2 = 0.0f;
		this->y2 = 0.0f;

		clicked = false;
	}

	void Draw_Box() {
		glColor3f(this->r, this->g, this->b);
		glRectf(this->x1, this->y1, this->x2, this->y2);
	}

	void new_rect() {
		GLclampf x1, y1;
		GLclampf new1 = (GLclampf)xy(eng), new2 = (GLclampf)xy(eng);
		x1 = new1;
		y1 = new2;

		this->x1 = x1;
		this->y1 = y1;
		this->x2 = x1 + 0.1f;
		this->y2 = y1 + 0.1f;

		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	void set_rect(float cx, float cy) {
		float dx = abs(this->x2 - this->x1)/2;
		float dy = abs(this->y2 - this->y1) / 2;
		this->x1 = cx - dx;
		this->y1 = cy - dy;
		this->x2 = cx + dx;
		this->y2 = cy + dy;
	}

	bool is_in_rect(float cx, float cy) {
		float x = (this->x2 - this->x1) / 2 + min(this->x2, this->x1);
		float y = (this->y2 - this->y1) / 2 + min(this->y2, this->y1);
		return sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y)) < (abs(this->x2 - this->x1) / 2);
	}

	bool is_on_other(Box other) { //수정할거면 네모 겹치는 범위 바꾸기
		float x = (this->x2 - this->x1) / 2 + min(this->x2, this->x1);
		float y = (this->y2 - this->y1) / 2 + min(this->y2, this->y1);
		float ox = (other.x2 - other.x1) / 2 + min(other.x2, other.x1);
		float oy = (other.y2 - other.y1) / 2 + min(other.y2, other.y1);
		return sqrt((ox - x) * (ox - x) + (oy - y) * (oy - y)) < (abs(this->x2 - this->x1));
	}

	void recolor() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	void box_merge(Box other) {
		this->x1 = min(this->x1,other.x1);
		this->y1 = min(this->y1, other.y1);
		this->x2 = max(this->x2, other.x2);
		this->y2 = max(this->y2, other.y2);

		this->recolor();
	}

private:
	GLclampf r, g, b;
	GLclampf x1, y1, x2, y2;
};

vector<Box> box_group;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid DragMouse(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);

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

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(DragMouse);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < box_group.size(); i++)
	{
		box_group[i].Draw_Box();
	}
	
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y)
{
	Box newbox;
	switch (key) {
	case 'a':
		if (box_group.size() >= 10) break;
		newbox.new_rect();
		box_group.push_back(newbox);
		break;
	case 'q':
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	bool loop_break = false;
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
		{
			for (int i = 0; i < box_group.size(); i++)
			{
				if (box_group[i].is_in_rect(cx, cy))
				{
					box_group[i].clicked = true;
				}
			}
		}
		else if (state == GLUT_UP)
		{
			for (int i = 0; i < box_group.size(); i++){
				if (box_group[i].clicked)
				{
					for (int j = 0; j < box_group.size(); j++) {
						if (i != j && box_group[i].is_on_other(box_group[j]))
						{
							box_group[i].box_merge(box_group[j]);
							box_group.erase(box_group.begin() + j);
							loop_break = true;
							break;
						}
					}
					if (loop_break) break;
				}
			}
			loop_break = false;
			for (int i = 0; i < box_group.size(); i++) box_group[i].clicked = false;
		}
	}

	glutPostRedisplay();
}

void DragMouse(int x, int y) {
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);

	for (int i = 0; i < box_group.size(); i++)
	{
		if (box_group[i].clicked)
		{
			box_group[i].set_rect(cx, cy);
		}
	}
	glutPostRedisplay();
}