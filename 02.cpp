#include <iostream>
#include <random>

//--- �ʿ��� ������� include
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
			four_box[n].set_rect(j - 1, -i, j, (1 - i)); //�ڽ� ��ġ �ʱ�ȭ
			n++;
		}
	}

	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMouseFunc(Mouse); //Ű���� �Է� �ݹ� �Լ�
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
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
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (cx < 0 && cy >= 0) // ����
		{
			cout << "����" << endl;
		}
		else if (cx >= 0 && cy >= 0) //����
		{
			cout << "����" << endl;
		}
		else if (cx < 0 && cy < 0) //�޾�
		{
			cout << "�޾�" << endl;
		}
		else if (cx >= 0 && cy < 0) //����
		{
			cout << "����" << endl;
		}
	}
		
}