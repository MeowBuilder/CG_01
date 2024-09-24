#include <iostream>
#include <random>

//--- �ʿ��� ������� include
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
	init_Box();
	glutDisplayFunc(drawScene); // ��� �Լ��� ����
	glutReshapeFunc(Reshape); // �ٽ� �׸��� �Լ� ����
	glutMouseFunc(Mouse); //Ű���� �Է� �ݹ� �Լ�
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glClearColor(0.3f, 0.3f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT); // ������ ������ ��ü�� ĥ�ϱ�
	// �׸��� �κ� ����: �׸��� ���� �κ��� ���⿡ ���Եȴ�.

	for (const auto& box : four_box)
	{
		Draw_Rect(box);
	}

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (x < 400 && y < 300) // ����
		{

		}
		else if (x >= 400 && y < 300 ) //����
		{

		}
		else if (x < 400 && y >= 300) //�޾�
		{

		}
		else if (x >= 400 && y >= 300) //����
		{

		}
	}
		
}

void Draw_Rect(Box box) {
	glColor3f(box.r, box.g, box.b);
	glRectf(box.x1, box.y1, box.x2, box.y2);
}