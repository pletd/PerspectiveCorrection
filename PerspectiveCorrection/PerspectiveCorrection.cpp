#include <fstream>
#include <Windows.h>
#include <iostream>
//#include "opencv2/opencv.hpp"

//using namespace cv;
using namespace std;
string TESTIMG;
int StartPos[4][2];
int endPositions[4][2];

struct TEST {
public:
	string filePath;
	int SPos[4][2];
	int EPos[4][2];

	void FillPos(int s[4][2], int e[4][2]) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 2; j++) {
				SPos[i][j] = s[i][j];
				EPos[i][j] = e[i][j];
			}
		}

	}

	void Parse() {
		TESTIMG = filePath;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 2; j++) {
				StartPos[i][j] = SPos[i][j];
				endPositions[i][j] = EPos[i][j];
			}
		}
	}
};

class Image {
	unsigned short int width;
	unsigned short int height;
	unsigned char** color;
	unsigned short int MAXCOLOR;

public:

	Image(ifstream& in) {
		string FileType;
		char tmp;
		int place = 0;
		//Read header
		in >> FileType;
		cout << " File type is " << FileType << endl;
		if (FileType != "P5") {
			throw runtime_error(" Wrong File Format ");
		}
		// SKIP END OF STRING
		in.read(&tmp, 1);

		//SKIP COMMENTS
		place = in.tellg();
		in.read(&tmp, 1);
		while (tmp == '#') {
			while (tmp != '\n') {
				cout << tmp;
				in.read(&tmp, 1);
			}
			cout << tmp;
			place = in.tellg();
			in.read(&tmp, 1);
		}

		in.seekg(place);
		in >> width >> height;
		cout << " File width and height: " << width << "*" << height << endl; // in.read(&tmp, 1);
		in >> MAXCOLOR; in.read(&tmp, 1);
		cout << " Max color: " << MAXCOLOR << endl;

		//ofstream ot("red.ppm", ios::binary);
		//ot << 'P' << '6' << '\n' << width << ' ' << height << '\n' << MAXCOLOR << '\n';

		//Read colors
		color = new unsigned char* [width];
		for (int i = 0; i < width; i++) {
			color[i] = new unsigned char[height];
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				in.read((char*)&color[j][i], 1);
			}
			//cout << " " << i;
		}
		cout << endl;
	}

	Image(Image* img) {
		this->width = img->width;
		this->height = img->height;
		this->MAXCOLOR = img->MAXCOLOR;
		this->color = new unsigned char* [width];
		for (int i = 0; i < width; i++) {
			this->color[i] = new unsigned char[height];
			for (int j = 0; j < height; j++) {
				this->color[i][j] = 0;
			}
		}
	}

	~Image() {
		for (int i = 0; i < width; i++) {
			delete[] color[i];
		}
		delete[] color;
	}

	void WriteImage(string imageName) {
		ofstream out((imageName), ios::binary);
		out << 'P' << '5' << '\n' << width << ' ' << height << '\n' << MAXCOLOR << '\n';
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				out.write((char*)&color[j][i], 1);
				//counter -= 1;
			}
		}
		out.close();
	}

	Image* ApplyMask(double** mask) { //соответствие начальная->конечная
		Image* res = new Image(this);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				/*int x = mask[0][0] * i + mask[1][0] * j + mask[2][0]; //TEST1
				int y = mask[0][1] * i + mask[1][1] * j + mask[2][1];*/
				int x = (mask[0][0] * i + mask[1][0] * j + mask[2][0]) / (mask[0][2] * i + mask[1][2] * j + mask[2][2]);
				int y = (mask[0][1] * i + mask[1][1] * j + mask[2][1]) / (mask[0][2] * i + mask[1][2] * j + mask[2][2]);
				if (x >= width || x < 0 || y >= height || y < 0) {

				}
				else {
					res->color[x][y] = color[i][j];
				}
				/*int x = (mask[0][0] * i + mask[1][0] * j + mask[2][0]) / (mask[0][2] * i + mask[1][2] * j + 1);
				int y = (mask[0][1] * i + mask[1][1] * j + mask[2][1]) / (mask[0][2] * i + mask[1][2] * j + 1);
				if(x<width&&x>=0&&y<height&&y>=0)
				res->color[i][j] = color[x][y];*/
			}
		}
		return res;
	}

	Image* ApplyMaskWI(double** mask) { //соотвествие конечная-> начальная (не требует инверсии Н и апроксимации пикселей)
		Image* res = new Image(this);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				int x = (mask[0][0] * i + mask[1][0] * j + mask[2][0]) / (mask[0][2] * i + mask[1][2] * j + mask[2][2]);
				int y = (mask[0][1] * i + mask[1][1] * j + mask[2][1]) / (mask[0][2] * i + mask[1][2] * j + mask[2][2]);
				if (x >= width || x < 0 || y >= height || y < 0) {
					res->color[i][j] = 255; //области лежащие за прежелаит изначального изображения
				}
				else {
					res->color[i][j] = color[x][y];
				}
			}
		}
		return res;
	}
};

void FindMAXEndPositions() {
	if (StartPos[0][0] < StartPos[1][0]) {
		endPositions[0][0] = endPositions[1][0] = StartPos[0][0];
	}
	else {
		endPositions[0][0] = endPositions[1][0] = StartPos[1][0];
	}
	if (StartPos[0][1] < StartPos[3][1]) {
		endPositions[0][1] = endPositions[3][1] = StartPos[0][1];
	}
	else {
		endPositions[0][1] = endPositions[3][1] = StartPos[3][1];
	}
	if (StartPos[1][1] < StartPos[2][1]) {
		endPositions[1][1] = endPositions[2][1] = StartPos[2][1];
	}
	else {
		endPositions[1][1] = endPositions[2][1] = StartPos[1][1];
	}
	if (StartPos[2][0] < StartPos[3][0]) {
		endPositions[2][0] = endPositions[3][0] = StartPos[3][0];
	}
	else {
		endPositions[2][0] = endPositions[3][0] = StartPos[2][0];
	}
}

void FindMINEndPositions() {
	if (StartPos[0][0] > StartPos[1][0]) {
		endPositions[0][0] = endPositions[1][0] = StartPos[0][0];
	}
	else {
		endPositions[0][0] = endPositions[1][0] = StartPos[1][0];
	}
	if (StartPos[0][1] > StartPos[3][1]) {
		endPositions[0][1] = endPositions[3][1] = StartPos[0][1];
	}
	else {
		endPositions[0][1] = endPositions[3][1] = StartPos[3][1];
	}
	if (StartPos[1][1] > StartPos[2][1]) {
		endPositions[1][1] = endPositions[2][1] = StartPos[2][1];
	}
	else {
		endPositions[1][1] = endPositions[2][1] = StartPos[1][1];
	}
	if (StartPos[2][0] > StartPos[3][0]) {
		endPositions[2][0] = endPositions[3][0] = StartPos[3][0];
	}
	else {
		endPositions[2][0] = endPositions[3][0] = StartPos[2][0];
	}
}
void PrintMatr(double** matr, int Size) {
	for (int i = 0; i < Size; i++) {
		for (int j = 0; j < Size; j++) {
			cout << " " << matr[j][i];
		}
		cout << endl;
	}
	cout << endl;
}

double** FindMaskOLD() {
	double** mask = new double* [3];
	for (int i = 0; i < 3; i++) {
		mask[i] = new double[3];
	}
	//mask[2][2] = 1;
	//mask[0][2] = mask[1][2] = 0;
	//for (int i = 0; i < 2; i++) {
	//	//TEST1
	//	mask[0][i] = (StartPos[2][i] - StartPos[0][i]) * (endPositions[1][1] - endPositions[0][1]) + (StartPos[1][i] - StartPos[0][i]) * (endPositions[0][1] - endPositions[2][1]);
	//	mask[0][i] /= ((endPositions[2][0] - endPositions[0][0]) * (endPositions[1][1] - endPositions[0][1]) + (endPositions[1][0] - endPositions[0][0]) * (endPositions[0][1] - endPositions[2][1]));
	//	mask[1][i] = (StartPos[1][i] - StartPos[0][i]) + mask[0][i] * (endPositions[0][0] - endPositions[1][0]);
	//	mask[1][i] /= (endPositions[1][1] - endPositions[0][1]);
	//	mask[2][i] = StartPos[0][i] - mask[0][i] * endPositions[0][0] - mask[1][i] * endPositions[0][1];
	//}
		//TEST2
		//Write consts
	mask[2][2] = 1;
	double matr[8][8];
	double vec[8];
	for (int i = 0; i < 4; i++) { //x ур
		matr[0][i] = endPositions[i][0];
		matr[1][i] = endPositions[i][1];
		matr[2][i] = 1;
		matr[3][i] = 0;
		matr[4][i] = 0;
		matr[5][i] = 0;
		matr[6][i] = -1 * StartPos[i][0] * endPositions[i][0];
		matr[7][i] = -1 * StartPos[i][0] * endPositions[i][1];
		vec[i] = StartPos[i][0];
	}
	for (int i = 0; i < 4; i++) { //y ур
		matr[0][i + 4] = 0;
		matr[1][i + 4] = 0;
		matr[2][i + 4] = 0;
		matr[3][i + 4] = endPositions[i][0];
		matr[4][i + 4] = endPositions[i][1];;
		matr[5][i + 4] = 1;
		matr[6][i + 4] = -1 * StartPos[i][1] * endPositions[i][0];
		matr[7][i + 4] = -1 * StartPos[i][1] * endPositions[i][1];
		vec[i + 4] = StartPos[i][1];
	}

	cout << endl;
	double temp;

	//Swap h21 and h31
	for (int i = 0; i < 8; i++) {
		temp = matr[3][i];
		matr[3][i] = matr[6][i];
		matr[6][i] = temp;
	}
	temp = vec[3];
	vec[3] = vec[6];
	vec[6] = temp;

	cout << " MATR DEB START" << endl;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			cout << " " << matr[j][i];
		}
		cout << vec[i] << endl;
	}

	//GAUSSS
	for (int i = 0; i < 7; i++) {
		for (int k = i + 1; k < 8; k++) {
			if (matr[i][i] != 0) {
				temp = matr[i][k] / matr[i][i];
				for (int j = 0; j < 8; j++) {
					matr[j][k] -= matr[j][i] * temp;
					//if (matr[j][k] < 0.0000001 && matr[j][k] > -0.0000001) matr[j][k] = 0;
				}
				vec[k] -= vec[i] * temp;
			}
		}
		cout << " MATR DEB ITER" << i << endl;
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				cout << " " << matr[j][i];
			}
			cout << vec[i] << endl;
		}
		cout << endl;
	}
	cout << " MATR DEB FINISH" << endl;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			cout << " " << matr[j][i];
		}
		cout << "  |  " << vec[i] << endl;
	}
	cout << endl;

	//FIND Hij;
	double res[8];
	for (int i = 7; i >= 0; i--) {
		for (int j = 7; j > i; j--) {
			vec[i] -= matr[j][i] * res[j];
		}
		res[i] = vec[i] / matr[i][i];
	}

	/*for (int i = 0; i < 8; i++) {
		cout << " " << res[i];
	}
	cout << endl;*/

	mask[0][0] = res[0];//h11
	mask[1][0] = res[1];//h12
	mask[2][0] = res[2];//h13
	mask[0][2] = res[3];//h31
	mask[1][1] = res[4];//h22
	mask[2][1] = res[5];//h23
	mask[0][1] = res[6];//h21
	mask[1][2] = res[7];//h32
	return mask;
}

double** Transpose(double** matr, int size) {
	double** mask = new double* [size];
	for (int i = 0; i < size; i++) {
		mask[i] = new double[size];
	}
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			mask[i][j] = matr[j][i];
		}
	}
	return mask;
}

double** MatrMult(double** A, double** B, int heightA, int widthA, int heightB, int widthB) {
	double** mask = new double* [widthB];
	for (int i = 0; i < widthB; i++) {
		mask[i] = new double[heightA];
		for (int j = 0; j < heightA; j++) {
			mask[i][j] = 0;
		}
	}
	for (int i = 0; i < heightA; i++) {
		for (int j = 0; j < widthB; j++) {
			for (int k = 0; k < widthA; k++) mask[j][i] += A[k][i] * B[j][k];
		}
	}
	return mask;
}

double** GetMatr(double** mas, int rows, int cols, int row, int col) {
	int di, dj;
	double** p = new double* [rows - 1];
	di = 0;
	for (int i = 0; i < rows - 1; i++) { // проверка индекса строки
		if (i == row)  // строка совпала с вычеркиваемой
			di = 1;    // - дальше индексы на 1 больше
		dj = 0;
		p[i] = new double[cols - 1];
		for (int j = 0; j < cols - 1; j++) { // проверка индекса столбца
			if (j == col)  // столбец совпал с вычеркиваемым
				dj = 1;    // - дальше индексы на 1 больше
			p[i][j] = mas[i + di][j + dj];
		}
	}
	return p;
}

double Determinant(double** mas, int m) {
	int k;
	double** p = 0;
	double d = 0;
	k = 1; //(-1) в степени i
	if (m < 1) { printf("Det error"); return 0; }
	if (m == 1) { d = mas[0][0]; return(d); }
	if (m == 2) { d = mas[0][0] * mas[1][1] - (mas[1][0] * mas[0][1]); return(d); }
	if (m > 2) {
		for (int i = 0; i < m; i++) {
			p = GetMatr(mas, m, m, i, 0);
			d = d + k * mas[i][0] * Determinant(p, m - 1);
			k = -k;
		}
	}
	return(d);
}

double** Invert(double** mas, int m)
{
	double** rez = new double* [m];
	double det = Determinant(mas, m); // находим определитель исходной матрицы
	for (int i = 0; i < m; i++)
	{
		rez[i] = new double[m];
		for (int j = 0; j < m; j++)
		{
			rez[i][j] = Determinant(GetMatr(mas, m, m, i, j), m - 1);
			if ((i + j) % 2 == 1)       // если сумма индексов строки и столбца нечетная
				rez[i][j] = -rez[i][j];    // меняем знак минора
			rez[i][j] = rez[i][j] / det;
		}
	}
	return Transpose(rez, m);
}

double** FindMatr() {
	double** mask = new double* [3];
	for (int i = 0; i < 3; i++) {
		mask[i] = new double[3];
	}
	double** matr = new double* [8];
	double** vec = new double* [1];
	vec[0] = new double[8];
	double** h = new double* [1];
	h[0] = new double[8];
	for (int i = 0; i < 8; i++) {
		matr[i] = new double[8];
	}
	//Fill
	for (int i = 0; i < 4; i++) {
		matr[0][i * 2] = endPositions[i][0]; //h11
		matr[1][i * 2] = endPositions[i][1]; //h12
		matr[2][i * 2] = 1;	//h13
		matr[3][i * 2] = 0;//h21
		matr[4][i * 2] = 0; //h22
		matr[5][i * 2] = 0;//h23
		matr[6][i * 2] = -1 * StartPos[i][0] * endPositions[i][0]; //h31
		matr[7][i * 2] = -1 * StartPos[i][0] * endPositions[i][1]; //h32
		vec[0][i * 2] = StartPos[i][0];

		matr[0][(i * 2 + 1)] = 0; //h11
		matr[1][(i * 2 + 1)] = 0; //h12
		matr[2][(i * 2 + 1)] = 0;	//h13
		matr[3][(i * 2 + 1)] = endPositions[i][0];//h21
		matr[4][(i * 2 + 1)] = endPositions[i][1]; //h22
		matr[5][(i * 2 + 1)] = 1;//h23
		matr[6][(i * 2 + 1)] = -1 * endPositions[i][0] * StartPos[i][1];//h31
		matr[7][(i * 2 + 1)] = -1 * StartPos[i][1] * endPositions[i][1]; //h32
		vec[0][(i * 2 + 1)] = StartPos[i][1]; //h33
	}
	double** tm = new double* [8];
	double** vm = new double* [1];
	vm[0] = new double[8];
	for (int i = 0; i < 8; i++) {
		tm[i] = new double[8];
	}
	/*PrintMatr(matr,8);
	PrintMatr(Transpose(matr, 8),8);
	PrintMatr(MatrMult(Transpose(matr, 8), matr, 8, 8, 8, 8), 8);
	PrintMatr(Invert(MatrMult(Transpose(matr, 8), matr, 8, 8, 8, 8), 8), 8);*/

	h = MatrMult(Invert(MatrMult(Transpose(matr, 8), matr, 8, 8, 8, 8), 8), MatrMult(Transpose(matr, 8), vec, 8, 8, 8, 1), 8, 8, 8, 1);
	mask[0][0] = h[0][0];
	mask[1][0] = h[0][1];
	mask[2][0] = h[0][2];
	mask[0][1] = h[0][3];
	mask[1][1] = h[0][4];
	mask[2][1] = h[0][5];
	mask[0][2] = h[0][6];
	mask[1][2] = h[0][7];
	mask[2][2] = 1;
	return mask;
}

double** FindMask() {
	double** mask = new double* [3];
	for (int i = 0; i < 3; i++) {
		mask[i] = new double[3];
	}
	double** matr = new double* [9];
	double* vec = new double[9];
	double* h = new double[9];
	for (int i = 0; i < 9; i++) {
		matr[i] = new double[9];
	}
	//Fill
	for (int i = 0; i < 8; i++) {
		vec[i] = 0;
		matr[i][8] = 0;
	}
	vec[8] = 1;
	matr[8][8] = 1;
	for (int i = 0; i < 4; i++) {
		//X
		matr[0][i] = endPositions[i][0]; //h11
		matr[1][i] = endPositions[i][1]; //h12
		matr[2][i] = 1;	//h13
		matr[3][i] = -1 * StartPos[i][0] * endPositions[i][0]; //h31
		matr[4][i] = 0; //h22
		matr[5][i] = 0;//h23
		matr[6][i] = 0;//21
		matr[7][i] = -1 * StartPos[i][0] * endPositions[i][1]; //h32
		matr[8][i] = -1 * StartPos[i][0]; //h33
		//Y
		matr[0][i + 4] = 0; //h11
		matr[1][i + 4] = 0; //h12
		matr[2][i + 4] = 0;	//h13
		matr[3][i + 4] = -1 * StartPos[i][1] * endPositions[i][0]; //h31
		matr[4][i + 4] = endPositions[i][1]; //h22
		matr[5][i + 4] = 1;//h23
		matr[6][i + 4] = 1 * endPositions[i][0];//21
		matr[7][i + 4] = -1 * StartPos[i][1] * endPositions[i][1]; //h32
		matr[8][i + 4] = -1 * StartPos[i][1]; //h33
	}
	//Gauss forward
	double temp;
	for (int i = 0; i < 8; i++) {
		for (int j = i + 1; j <= 8; j++) {
			temp = matr[i][j] / matr[i][i];
			for (int k = i; k <= 8; k++) {
				matr[k][j] -= temp * matr[k][i];
			}
		}
	}
	/*cout << "Gauss forward" << endl;
	PrintMatr(matr, 9);
	for (int i = 0; i < 9; i++) {
		cout << " " << vec[i];
	}
	cout << endl;*/
	//Gauss backward
	for (int i = 8; i > 0; i--) {
		for (int j = i - 1; j >= 0; j--) {
			temp = matr[i][j] / matr[i][i];
			for (int k = i; k >= 0; k--) {
				matr[k][j] -= temp * matr[k][i];
			}
			vec[j] -= vec[i] * temp;
		}
	}
	/*cout << "Gauss backward" << endl;
	PrintMatr(matr, 9);
	for (int i = 0; i < 9; i++) {
		cout << " " << vec[i];
	}
	cout << endl;*/

	for (int i = 0; i < 9; i++) {
		h[i] = vec[i] / matr[i][i];
	}

	mask[0][0] = h[0];
	mask[1][0] = h[1];
	mask[2][0] = h[2];
	mask[0][2] = h[3];
	mask[1][1] = h[4];
	mask[2][1] = h[5];
	mask[0][1] = h[6];
	mask[1][2] = h[7];
	mask[2][2] = h[8];

	return mask;
}

void readFromConsole() {
	cout << " Enter path to image " << endl;
	cin >> TESTIMG;
	cout << " Enter start positions (4 points in format x y against clocwise starting from upper left corner)" << endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			cin >> StartPos[i][j];
		}
	}
	cout << " Chose end positions " << endl;
	int chose;
	cout << " 1-Max rectangle 2-Min rectangle 3-custom" << endl;
	cin >> chose;
	do {
		switch (chose) {
		case 1: FindMAXEndPositions(); break;
		case 2: FindMINEndPositions(); break;
		case 3: cout << " Enter end positions (4 points in format x y against clocwise starting from upper left corner)" << endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 2; j++) {
					cin >> endPositions[i][j];
				}
			}
			break;
		default: cout << " Wrong option "; break;
		}
	} while (chose < 1 || chose > 3);
}

void CorrectPerspectiveAndDisplay(string resFileName) {
	ifstream input(TESTIMG, ios::binary);
	if (!(input.is_open())) {
		cout << " Openning File " << TESTIMG << " Error " << endl;
		return;
	}
	cout << " Loading image " << endl;
	Image img(input);
	input.close();
	string openImage = "start " + TESTIMG;
	system(openImage.c_str());
	cout << " Finding mask " << endl;
	double** mask = FindMask();
	cout << " Applying mask " << endl;
	Image* res = img.ApplyMaskWI(mask);
	cout << " Writing image " << endl;
	res->WriteImage(resFileName);
	openImage = "start " + resFileName;
	system(openImage.c_str());
}

int main()
{
	string resFileName;
	//Test 1 sob max
	TEST test1;
	int st1[4][2] = { {300,170} ,{25,2180}, {1630,2170}, {1300,210} };
	int et1[4][2] = { {25,170} ,{25,2180}, {1630,2180}, {1630,170} };
	test1.filePath = "TESTIMAGE.pgm";
	test1.FillPos(st1, et1);
	//Test 2 sob min	
	TEST test2;
	int st2[4][2] = { {300,170} ,{25,2180}, {1630,2170}, {1300,210} };
	int et2[4][2] = { {300,210} ,{300,2170}, {1300,2170}, {1300,210} };
	test2.filePath = "TESTIMAGE.pgm";
	test2.FillPos(st2, et2);
	//Test 3 sob max
	TEST test3;
	int st3[4][2] = { {390,250} ,{160,2180}, {1520,2170}, {1310,270} };
	int et3[4][2] = { {160,100}, {160,2180}, {1520,2180}, {1520,100} };
	test3.filePath = "TESTIMAGE.pgm";
	test3.FillPos(st3, et3);
	//Test 4 tall coorect
	TEST test4;
	int st4[4][2] = { {110,40} ,{90,410}, {195,406},{200,40} };
	int et4[4][2] = { {110,40} ,{110,406}, {195,406}, {195,40} };
	test4.filePath = "Shift_Correction.pgm";
	test4.FillPos(st4, et4);
	//Test 5 wide paining correct
	TEST test5;
	int st5[4][2] = { {1858,582} ,{1860,1050}, {2330,1260},{2326,404} };
	int et5[4][2] = { {1350,404} ,{1350,1260}, {2330,1260}, {2326,404} };
	test5.filePath = "Painting.pgm";
	test5.FillPos(st5, et5);
	//Test 6
	TEST test6;
	int st6[4][2] = { {352,556} ,{960,1000}, {1410,740},{795,340} };
	int et6[4][2] = { {660,200} ,{660,1000}, {1260,1000}, {1260,200} };
	test6.filePath = "Book.pgm";
	test6.FillPos(st6, et6);


	int chose;
	do {
		cout << " Chose test or enter data " << endl << " 1-6 - tests 7 - enter 0 - exit " << endl;
		cin >> chose;
		switch (chose)
		{
		case 1: test1.Parse(); cout << " Test 1 " << endl; CorrectPerspectiveAndDisplay("ResTest1"); break;
		case 2: test2.Parse(); cout << " Test 2 " << endl; CorrectPerspectiveAndDisplay("ResTest2"); break;
		case 3: test3.Parse(); cout << " Test 3 " << endl; CorrectPerspectiveAndDisplay("ResTest3"); break;
		case 4: test4.Parse(); cout << " Test 4 " << endl; CorrectPerspectiveAndDisplay("ResTest4"); break;
		case 5: test5.Parse(); cout << " Test 5 " << endl; CorrectPerspectiveAndDisplay("ResTest5"); break;
		case 6: test6.Parse(); cout << " Test 6 " << endl; CorrectPerspectiveAndDisplay("ResTest6"); break;
		case 7: readFromConsole(); cout << " Enter res file name " << endl; cin >> resFileName; CorrectPerspectiveAndDisplay(resFileName); break;
		case 0: cout << " Exiting " << endl; break;
		default: cout << " Wrong command " << endl; break;
		}
	} while (chose != 0);

	return 0;
}