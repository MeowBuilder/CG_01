#include <iostream>
#include <random>

//--- �ʿ��� ������� include
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>

using namespace std;

random_device seeder;
const auto seed = seeder.entropy() ? seeder() : time(nullptr);
mt19937 eng(static_cast<mt19937::result_type>(seed));
uniform_real_distribution<double> dist(0.0f, 1.0f);

class Box
{
public:
	int num;

	Box() {
		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);

		this->x1 = 0.0f;
		this->y1 = 0.0f;
		this->x2 = 0.0f;
		this->y2 = 0.0f;
		this->num = 0;
	}

	void Draw_Box() {
		glColor3f(this->r, this->g, this->b);
		glRectf(this->x1, this->y1, this->x2, this->y2);
	}

	void set_rect(float x1, float y1, float x2, float y2,int num) {
		this->x1 = x1;
		this->y1 = y1;
		this->x2 = x2;
		this->y2 = y2;
		this->num = num;
	}

	bool is_in_rect(float cx, float cy) {
		if (cx < max(this->x2, this->x1) && cx >= min(this->x2, this->x1))
		{
			if (cy < max(this->y2, this->y1) && cy >= min(this->y2, this->y1))
			{
				return true;
			}
		}

		return false;
	}

	void resize(float d) {
		if (abs(this->x2 - this->x1) < 0.1f && d < 0)
		{
			return;
		}

		if (this->x2 > this->x1)
		{
			this->x1 -= d / 2;
			this->x2 += d / 2;
		}
		else
		{
			this->x2 -= d / 2;
			this->x1 += d / 2;
		}

		if (this->y2 > this->y1)
		{
			this->y1 -= d / 2;
			this->y2 += d / 2;
		}
		else
		{
			this->y2 -= d / 2;
			this->y1 += d / 2;
		}
	}

	void recolor() {
		random_device seeder;
		const auto seed = seeder.entropy() ? seeder() : time(nullptr);
		mt19937 eng(static_cast<mt19937::result_type>(seed));
		uniform_real_distribution<double> dist(0.0f, 1.0f);

		this->r = (GLclampf)dist(eng);
		this->g = (GLclampf)dist(eng);
		this->b = (GLclampf)dist(eng);
	}
private:
	GLclampf r, g, b;
	GLclampf x1, y1, x2, y2;
};

Box four_box[4];

GLclampf r, g, b;

void bgrecolor() {
	r = (GLclampf)dist(eng);
	g = (GLclampf)dist(eng);
	b = (GLclampf)dist(eng);
}

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);

GLclampf convert_to_clampf_X(int x, int w) {
	return (x - (w / 2.0f)) / (w / 2.0f);
}
GLclampf convert_to_clampf_Y(int y, int h) {
	return (y - (h / 2.0f)) / (-h / 2.0f);
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{ //--- ������ �����ϱ�
	glutInit(&argc, argv); // glut �ʱ�ȭ
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���÷��� ��� ����
	glutInitWindowPosition(0, 0); // �������� ��ġ ����
	glutInitWindowSize(800, 600); // �������� ũ�� ����
	glutCreateWindow("Example1"); // ������ ����(������ �̸�)

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) // glew �ʱ�ȭ
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
			four_box[n].set_rect(j - 1, -i, j, (1 - i),n); //�ڽ� ��ġ �ʱ�ȭ
			n++;
		}
	}
	bgrecolor();
	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMouseFunc(Mouse); //Ű���� �Է� �ݹ� �Լ�
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glClearColor(r, g, b, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.

	for (int i = 0; i < 4; i++)
	{
		four_box[i].Draw_Box();
	}

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	GLclampf cx = convert_to_clampf_X(x, 800);
	GLclampf cy = convert_to_clampf_Y(y, 600);
	if (state == GLUT_DOWN) {
		if (cx < 0 && cy >= 0) // ����
		{
			cout << "����" << endl;
			
			if (button == GLUT_LEFT_BUTTON)
			{
				if (four_box[0].is_in_rect(cx, cy))
				{
					four_box[0].recolor();
				}
				else
				{
					bgrecolor();
				}
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				if (four_box[0].is_in_rect(cx, cy))
				{
					four_box[0].resize(-0.1f);
				}
				else
				{
					four_box[0].resize(0.1f);
				}
			}
		}
		else if (cx >= 0 && cy >= 0) //����
		{
			cout << "����" << endl;
			if (button == GLUT_LEFT_BUTTON)
			{
				if (four_box[1].is_in_rect(cx, cy))
				{
					four_box[1].recolor();
				}
				else
				{
					bgrecolor();
				}
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				if (four_box[1].is_in_rect(cx, cy))
				{
					four_box[1].resize(-0.1f);
				}
				else
				{
					four_box[1].resize(0.1f);
				}
			}
		}
		else if (cx < 0 && cy < 0) //�޾�
		{
			cout << "�޾�" << endl;
			if (button == GLUT_LEFT_BUTTON)
			{
				if (four_box[2].is_in_rect(cx, cy))
				{
					four_box[2].recolor();
				}
				else
				{
					bgrecolor();
				}
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				if (four_box[2].is_in_rect(cx, cy))
				{
					four_box[2].resize(-0.1f);
				}
				else
				{
					four_box[2].resize(0.1f);
				}
			}
		}
		else if (cx >= 0 && cy < 0) //����
		{
			cout << "����" << endl;
			if (button == GLUT_LEFT_BUTTON)
			{
				if (four_box[3].is_in_rect(cx, cy))
				{
					four_box[3].recolor();
				}
				else
				{
					bgrecolor();
				}
			}
			else if (button == GLUT_RIGHT_BUTTON)
			{
				if (four_box[3].is_in_rect(cx, cy))
				{
					four_box[3].resize(-0.1f);
				}
				else
				{
					four_box[3].resize(0.1f);
				}
			}
		}
	}

	glutPostRedisplay();
}