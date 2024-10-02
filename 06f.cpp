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
uniform_real_distribution<double> dxy(0.05f, 0.5f);
uniform_int_distribution<int> no6(1, 4);

class Box
{
public:
	GLclampf x1, y1, x2, y2;
	Box() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);

		this->x1 = 0.0f;
		this->y1 = 0.0f;
		this->x2 = 0.0f;
		this->y2 = 0.0f;

		xspeed = 0;
		yspeed = 0;
	}

	void Draw_Box() {
		glColor3f(this->r, this->g, this->b);
		glRectf(this->x1, this->y1, this->x2, this->y2);
	}

	void new_rect() {
		GLclampf x1 = (GLclampf)xy(eng), y1 = (GLclampf)xy(eng),dx = (GLclampf)dxy(eng),dy = (GLclampf)dxy(eng);

		this->x1 = x1 - (dx / 2);
		this->y1 = y1 - (dy / 2);
		this->x2 = x1 + (dx / 2);
		this->y2 = y1 + (dy / 2);

		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	void move_rect() {
		this->x1 += xspeed;
		this->y1 += yspeed;
		this->x2 += xspeed;
		this->y2 += yspeed;

		if ((xspeed != 0 || yspeed != 0) && !this->is_destroy())
		{
			this->x1 += 0.005f;
			this->y1 += 0.005f;
			this->x2 -= 0.005f;
			this->y2 -= 0.005f;
		}

		if (this->x1 < -1.0f || this->x2 > 1.0f)
		{
			xspeed = -xspeed;
		}
		else if (this->y1 < -1.0f || this->y2 > 1.0f)
		{
			yspeed = -yspeed;
		}
	}

	void split_rect(Box origin) {
		float origin_mx = origin.x1 + ((origin.x2 - origin.x1) / 2);
		float origin_my = origin.y1 + ((origin.y2 - origin.y1) / 2);
		float origin_dx = ((origin.x2 - origin.x1) / 4);
		float origin_dy = ((origin.y2 - origin.y1) / 4);
		this->x1 = origin_mx - origin_dx;
		this->x2 = origin_mx + origin_dx;
		this->y1 = origin_my - origin_dy;
		this->y2 = origin_my + origin_dy;
	}

	bool is_in_rect(float cx, float cy) {
		float x = (this->x2 - this->x1) / 2 + min(this->x2, this->x1);
		float y = (this->y2 - this->y1) / 2 + min(this->y2, this->y1);
		return sqrt((cx - x) * (cx - x) + (cy - y) * (cy - y)) < abs(this->x2 - this->x1);
	}

	void set_speed(float xspeed, float yspeed) {
		this->xspeed = xspeed;
		this->yspeed = yspeed;
	}

	void recolor() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	bool is_destroy() {
		if (this->x2 - this->x1 < 0.01f || this->y2 - this->y1 < 0.01f) return true;
		else return false;
	}

private:
	GLclampf r, g, b;
	float xspeed, yspeed;
};

vector<Box> box_group;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid DragMouse(int x, int y);
GLvoid TimerFunction(int value);

int move_count = 0;
int move_way = 0;
bool move_timer1 = false, move_timer2 = false;
bool shape_timer = false, color_timer = false;

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

	Box newbox;
	box_group.clear();
	for (int i = 0; i < 10; i++)
	{
		newbox.new_rect();
		box_group.push_back(newbox);
	}

	glutTimerFunc(100, TimerFunction, 0);
	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutMouseFunc(Mouse);
	glutMainLoop(); // 이벤트 처리 시작
}

GLvoid drawScene()
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

void Mouse(int button, int state, int x, int y)
{
	Box newbox1, newbox2, newbox3, newbox4, newbox5, newbox6, newbox7, newbox8;
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN)
		{
			for (int i = 0; i < box_group.size(); i++)
			{
				if (box_group[i].is_in_rect(cx,cy))
				{
					int a = no6(eng);
					switch (a)
					{
					case 1://좌우상하
						newbox1.split_rect(box_group[i]);
						newbox1.set_speed(-0.1f, 0.0f);
						box_group.push_back(newbox1);
						newbox2.split_rect(box_group[i]);
						newbox2.set_speed(0.1f, 0.0f);
						box_group.push_back(newbox2);
						newbox3.split_rect(box_group[i]);
						newbox3.set_speed(0.0f, 0.1f);
						box_group.push_back(newbox3);
						newbox4.split_rect(box_group[i]);
						newbox4.set_speed(0.0f, -0.1f);
						box_group.push_back(newbox4);
						break;
					case 2://대각선
						newbox1.split_rect(box_group[i]);
						newbox1.set_speed(-0.1f, 0.1f);
						box_group.push_back(newbox1);
						newbox2.split_rect(box_group[i]);
						newbox2.set_speed(0.1f, 0.1f);
						box_group.push_back(newbox2);
						newbox3.split_rect(box_group[i]);
						newbox3.set_speed(-0.1f, -0.1f);
						box_group.push_back(newbox3);
						newbox4.split_rect(box_group[i]);
						newbox4.set_speed(0.1f, -0.1f);
						box_group.push_back(newbox4);
						break;
					case 3://사등분 한쪽이동
						newbox1.split_rect(box_group[i]);
						newbox1.set_speed(0.1f, 0.0f);
						box_group.push_back(newbox1);
						newbox2.split_rect(box_group[i]);
						newbox2.set_speed(0.2f, 0.0f);
						box_group.push_back(newbox2);
						newbox3.split_rect(box_group[i]);
						newbox3.set_speed(0.3f, 0.0f);
						box_group.push_back(newbox3);
						newbox4.split_rect(box_group[i]);
						newbox4.set_speed(0.4f, 0.0f);
						box_group.push_back(newbox4);

						break;
					case 4://좌우상하대각선 8등분
						newbox1.split_rect(box_group[i]);
						newbox1.set_speed(-0.1f, 0.0f);
						box_group.push_back(newbox1);
						newbox2.split_rect(box_group[i]);
						newbox2.set_speed(0.1f, 0.0f);
						box_group.push_back(newbox2);
						newbox3.split_rect(box_group[i]);
						newbox3.set_speed(0.0f, 0.1f);
						box_group.push_back(newbox3);
						newbox4.split_rect(box_group[i]);
						newbox4.set_speed(0.0f, -0.1f);
						box_group.push_back(newbox4);
						newbox5.split_rect(box_group[i]);
						newbox5.set_speed(-0.1f, 0.1f);
						box_group.push_back(newbox5);
						newbox6.split_rect(box_group[i]);
						newbox6.set_speed(0.1f, 0.1f);
						box_group.push_back(newbox6);
						newbox7.split_rect(box_group[i]);
						newbox7.set_speed(-0.1f, -0.1f);
						box_group.push_back(newbox7);
						newbox8.split_rect(box_group[i]);
						newbox8.set_speed(0.1f, -0.1f);
						box_group.push_back(newbox8);
						break;
					default:
						break;
					}
					box_group.erase(box_group.begin() + i);
					break;
				}
			}
		}
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	for (int j = 0; j < box_group.size(); j++) {
		box_group[j].move_rect();
		if (box_group[j].is_destroy())
		{
			box_group.erase(box_group.begin() + j);
		}
	}
	glutTimerFunc(100, TimerFunction, 0);
	glutPostRedisplay();
}
