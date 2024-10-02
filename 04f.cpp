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
uniform_real_distribution<double> dxy(-0.05f, 0.05f);

class Box
{
public:
	Box() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);

		this->x1 = 0.0f;
		this->y1 = 0.0f;
		this->x2 = 0.0f;
		this->y2 = 0.0f;

		this->sx1 = 0.0f;
		this->sy1 = 0.0f;
		this->sx2 = 0.0f;
		this->sy2 = 0.0f;

		xspeed = 0;
		yspeed = 0;
	}

	void Draw_Box() {
		glColor3f(this->r, this->g, this->b);
		glRectf(this->x1, this->y1, this->x2, this->y2);
	}

	void new_rect(float cx, float cy) {
		this->x1 = cx - 0.05f;
		this->y1 = cy - 0.05f;
		this->x2 = cx + 0.05f;
		this->y2 = cy + 0.05f;

		this->sx1 = this->x1;
		this->sy1 = this->y1;
		this->sx2 = this->x2;
		this->sy2 = this->y2;

		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	void move_rect() {
		this->x1 += xspeed;
		this->y1 += yspeed;
		this->x2 += xspeed;
		this->y2 += yspeed;

		if (this->x1 < -1.0f || this->x2 > 1.0f)
		{
			xspeed = -xspeed;
		}
		else if (this->y1 < -1.0f || this->y2 > 1.0f)
		{
			yspeed = -yspeed;
		}
	}

	void move_only(bool is_y) {
		if (is_y)
		{
			this->y1 += yspeed;
			this->y2 += yspeed;

			if (this->y1 < -1.0f || this->y2 > 1.0f)
			{
				yspeed = -yspeed;
			}
		}
		else
		{
			this->x1 += xspeed;
			this->x2 += xspeed;

			if (this->x1 < -1.0f || this->x2 > 1.0f)
			{
				xspeed = -xspeed;
			}
		}
	}

	void set_speed(float xspeed, float yspeed) {
		this->xspeed = xspeed;
		this->yspeed = yspeed;
	}

	void x_flip() {
		this->xspeed = -xspeed;
	}

	void y_flip() {
		this->yspeed = -yspeed;
	}

	void resize(float dx,float dy) {
		if (abs(this->x2 - this->x1) < 0.01f && dx < 0)
		{
			return;
		}
		else if (abs(this->y2 - this->y1) < 0.01f && dy < 0)
		{
			return;
		}

		if (this->x2 > this->x1)
		{
			this->x1 -= dx / 2;
			this->x2 += dx / 2;
		}
		else
		{
			this->x2 -= dx / 2;
			this->x1 += dx / 2;
		}

		if (this->y2 > this->y1)
		{
			this->y1 -= dy / 2;
			this->y2 += dy / 2;
		}
		else
		{
			this->y2 -= dy / 2;
			this->y1 += dy / 2;
		}
	}

	void recolor() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}

	void to_start() {
		this->x1 = this->sx1;
		this->y1 = this->sy1;
		this->x2 = this->sx2;
		this->y2 = this->sy2;
	}

private:
	GLclampf r, g, b;
	GLclampf x1, y1, x2, y2;
	GLclampf sx1, sy1, sx2, sy2;
	float xspeed, yspeed;
};

vector<Box> box_group;

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid DragMouse(int x, int y);
GLvoid Keyboard(unsigned char key, int x, int y);
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

	glutDisplayFunc(drawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape); // 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);
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

void Keyboard(unsigned char key, int x, int y)
{
	Box newbox;
	switch (key) {
	case '1':
		for (int i = 0; i < box_group.size(); i++)
		{
			box_group[i].set_speed(0.01f,0.01f);
		}

		if (!move_timer1)
		{
			glutTimerFunc(10, TimerFunction, 1);
			move_timer1 = true;
		}
		else
		{
			move_timer1 = false;
		}
		break;
	case '2':
		for (int i = 0; i < box_group.size(); i++)
		{
			box_group[i].set_speed(-0.01f, 0.01f);
			move_way = 0;
		}

		if (!move_timer2)
		{
			glutTimerFunc(10, TimerFunction, 2);
			move_timer2 = true;
		}
		else
		{
			move_timer2 = false;
		}
		break;
	case '3':
		if (!shape_timer)
		{
			glutTimerFunc(10, TimerFunction, 3);
			shape_timer = true;
		}
		else
		{
			shape_timer = false;
		}
		break;
	case '4':
		if (!color_timer)
		{
			glutTimerFunc(10, TimerFunction, 4);
			color_timer = true;
		}
		else
		{
			color_timer = false;
		}
		break;
	case 's':
		move_timer1 = false;
		move_timer2 = false;
		shape_timer = false;
		color_timer = false;
		break;
	case 'm':
		for (int i = 0; i < box_group.size(); i++)
		{
			box_group[i].to_start();
		}
		break;
	case 'r':
		box_group.clear();
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
			if (!(box_group.size() >= 5))
			{
				Box newbox;
				newbox.new_rect(cx, cy);
				box_group.push_back(newbox);
			}
		}
	}

	glutPostRedisplay();
}

void TimerFunction(int value)
{
	switch (value)
	{
	case 1:
		for (int i = 0; i < box_group.size(); i++)
		{
			box_group[i].move_rect();
		}
		if (move_timer1) glutTimerFunc(10, TimerFunction, value);
		break;
	case 2:
		move_count++;
		//방향바꾸기
		if (move_count > 50) 
		{
			move_count = 0;
			move_way = (move_way + 1) % 4;
			for (int i = 0; i < box_group.size(); i++)
			{
				switch (move_way)
				{
				case 0:
					box_group[i].x_flip();
					break;
				case 1:
				case 3:
					break;
				case 2:
					box_group[i].x_flip();
					break;
				default:
					break;
				}
			}
		}
		//이동
		for (int i = 0; i < box_group.size(); i++)
		{
			switch (move_way)
			{
			case 0:
				box_group[i].move_only(false);
				break;
			case 1:
			case 3:
				box_group[i].move_only(true);
				break;
			case 2:
				box_group[i].move_only(false);
				break;
			default:
				break;
			}
		}
		if (move_timer2) glutTimerFunc(10, TimerFunction, value);
		break;
	case 3:
		for (int i = 0; i < box_group.size(); i++){
			box_group[i].resize(dxy(eng), dxy(eng));
		}
		if (shape_timer) glutTimerFunc(10, TimerFunction, value);
		break;
	case 4:
		for (int i = 0; i < box_group.size(); i++) {
			box_group[i].recolor();
		}
		if (color_timer) glutTimerFunc(10, TimerFunction, value);
		break;
	default:
		break;
	}


	glutPostRedisplay();
}