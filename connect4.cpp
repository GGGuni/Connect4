#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

void dummy(int, void*)
{

}
int threshold1 = 30;

struct playerInfo
{
	char playerName[81];
	char playerID;
};

int PlayerDrop(char board[][10], playerInfo activePlayer);
void CheckBellow(char board[][10], playerInfo activePlayer, int dropChoice);
void DisplayBoard(char board[][10]);
int CheckFour(char board[][10], playerInfo activePlayer);
int FullBoard(char board[][10]);
void PlayerWin(playerInfo activePlayer);
int restart(char board[][10]);

int main()
{
	Mat img_color;
	Mat hsv_color;

	VideoCapture cap(0);
	if (!cap.isOpened()) {

		cout << "카메라를 열 수 없습니다." << endl;
		return -1;
	}

	namedWindow("img_color");

	int hue = 6;
	int saturation = 70;
	int value = 70;

	cout << "hue = " << hue << endl;
	cout << "saturation = " << saturation << endl;
	cout << "value = " << value << endl;

	Vec3b lower_red1, upper_red1; //170~180, 0~10
	Vec3b lower_red2, upper_red2;

	Vec3b lower_blue, upper_blue;//0~180


	createTrackbar("threshold", "img_color", &threshold1, 255, dummy);
	setTrackbarPos("threshold", "img_color", 30);

	playerInfo playerOne, playerTwo;

	char board[9][10];
	int trueWidth = 7;
	int trueLength = 6;
	int dropChoice, win, full, again;

	cout << "입체 사목 게임 시작!" << endl << endl;
	cout << "플레이어 1 이름 : ";
	cin >> playerOne.playerName;
	playerOne.playerID = 'X';
	cout << "플레이어 2 이름 : ";
	cin >> playerTwo.playerName;
	playerTwo.playerID = 'O';

	full = 0;
	win = 0;
	again = 0;

	do
	{
		cap.read(img_color);

		cvtColor(img_color, hsv_color, COLOR_BGR2HSV);

		lower_blue = Vec3b(110 - hue, 70, 55);
		upper_blue = Vec3b(110 + hue, 255, 255);
		lower_red1 = Vec3b(10 - hue, 100, 100);
		upper_red1 = Vec3b(10 + hue, 255, 255);
		lower_red2 = Vec3b(170 - hue, 100, 100);
		upper_red2 = Vec3b(170 + hue, 255, 255);

		Mat img_mask1, img_mask2, img_mask3, img_mask;
		inRange(hsv_color, lower_blue, upper_blue, img_mask1);
		inRange(hsv_color, lower_red1, upper_red1, img_mask2);
		inRange(hsv_color, lower_red2, upper_red2, img_mask3);
		img_mask = img_mask1 | img_mask2 | img_mask3;

		threshold1 = getTrackbarPos("threshold", "img_color");

		rectangle(img_color, Point(0, 0), Point(213.3, 240), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(213.3, 0), Point(426.6, 240), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(426.6, 0), Point(640, 240), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(0, 240), Point(160, 480), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(160, 240), Point(320, 480), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(320, 240), Point(480, 480), Scalar(255, 0, 0), 3, 1, 0);
		rectangle(img_color, Point(480, 240), Point(640, 480), Scalar(255, 0, 0), 3, 1, 0);

		Rect roi[7];
		Rect roi_1 (Point(0, 0), Point(213.3, 240));
		Rect roi_2(Point(213.3, 0), Point(426.6, 240));
		Rect roi_3(Point(426.6, 0), Point(640, 240));
		Rect roi_4(Point(0, 240), Point(160, 480));
		Rect roi_5(Point(160, 240), Point(320, 480));
		Rect roi_6(Point(320, 240), Point(480, 480));
		Rect roi_7(Point(480, 240), Point(640, 480));

		int num1 = countNonZero(img_mask(roi_1));
		int num2 = countNonZero(img_mask(roi_2));
		int num3 = countNonZero(img_mask(roi_3));
		int num4 = countNonZero(img_mask(roi_4));
		int num5 = countNonZero(img_mask(roi_5));
		int num6 = countNonZero(img_mask(roi_6));
		int num7 = countNonZero(img_mask(roi_7));
		int num;

		if (num1 > 100) num=1;
		else if (num2 > 100) num=2;
		else if (num3 > 100) num = 3;
		else if (num4 > 100) num = 4;
		else if (num5 > 100) num = 5;
		else if (num6 > 100) num = 6;
		else if (num7 > 100) num = 7;


		//모폴로지
		int morph_size = 2;
		Mat element = getStructuringElement(MORPH_RECT, Size(2 * morph_size + 1, 2 * morph_size + 1),
		Point(morph_size, morph_size));
		morphologyEx(img_mask, img_mask, MORPH_OPEN, element);
		morphologyEx(img_mask, img_mask, MORPH_CLOSE, element);

		Mat img_result;
		bitwise_and(img_color, img_color, img_result, img_mask);

		//라벨링
		Mat img_labels, stats, centroids;
		int numOfLabels = connectedComponentsWithStats(img_mask, img_labels, stats, centroids, 8, CV_32S);
		for (int j = 1; j < numOfLabels; j++)
		{
			int area = stats.at<int>(j, CC_STAT_AREA);
			int left = stats.at<int>(j, CC_STAT_LEFT);
			int top = stats.at<int>(j, CC_STAT_TOP);
			int width = stats.at<int>(j, CC_STAT_WIDTH);
			int height = stats.at<int>(j, CC_STAT_HEIGHT);

			int centerX = centroids.at<double>(j, 0);
			int centerY = centroids.at<double>(j, 1);
			if (area > 100)
			{
				circle(img_color, Point(centerX, centerY), 5, Scalar(255, 0, 0), 1);
				rectangle(img_color, Point(left, top), Point(left + width, top + height), Scalar(0, 0, 255), 1);
			}
		}


		imshow("img_color", img_color);
		imshow("img_mask", img_mask);
		imshow("img_result", img_result);

		//dropChoice = PlayerDrop(board, playerOne);
		CheckBellow(board, playerOne, num);
		DisplayBoard(board);
		win = CheckFour(board, playerOne);
		if (win == 1)
		{
			PlayerWin(playerOne);
			again = restart(board);
			if (again == 2)
			{
				break;
			}
		}

		//dropChoice = PlayerDrop(board, playerTwo);
		CheckBellow(board, playerTwo, num);
		DisplayBoard(board);
		win = CheckFour(board, playerTwo);
		if (win == 1)
		{
			PlayerWin(playerTwo);
			again = restart(board);
			if (again == 2)
			{
				break;
			}
		}

		full = FullBoard(board);
		if (full == 7)
		{
			cout << "The board is full, it is a draw!" << endl;
			again = restart(board);
		}
		//if (waitKey(1) > 0)
			//break;
	} while (again!=2);


	return 0;
}


/*int PlayerDrop(char board[][10], playerInfo activePlayer)
{
	int dropChoice;
	do
	{
		cout << activePlayer.playerName << "차례";
		cout << " 1 ~ 7 사이의 숫자를 입력하세요 : ";
		cin >> dropChoice;

		while (board[1][dropChoice] == 'X' || board[1][dropChoice] == 'O')
		{
			cout << "그 줄은 가득 찼습니다. 다른 줄을 선택해주세요 :";
			cin >> dropChoice;
		}

	} while (dropChoice < 1 || dropChoice > 7);

	return dropChoice;
}*/

void CheckBellow(char board[][10], playerInfo activePlayer, int dropChoice)
{
	int length, turn;
	length = 6;
	turn = 0;

	do
	{
		if (board[length][dropChoice] != 'X' && board[length][dropChoice] != 'O')
		{
			board[length][dropChoice] = activePlayer.playerID;
			turn = 1;
		}
		else
			--length;
	} while (turn != 1);


}

void DisplayBoard(char board[][10])
{
	int rows = 6, columns = 7, i, ix;

	for (i = 1; i <= rows; i++)
	{
		cout << "|";
		for (ix = 1; ix <= columns; ix++)
		{
			if (board[i][ix] != 'X' && board[i][ix] != 'O')
				board[i][ix] = '*';

			cout << board[i][ix];

		}

		cout << "|" << endl;
	}

}

int CheckFour(char board[][10], playerInfo activePlayer)
{
	char XO;
	int win;

	XO = activePlayer.playerID;
	win = 0;

	for (int i = 8; i >= 1; --i)
	{

		for (int ix = 9; ix >= 1; --ix)
		{

			if (board[i][ix] == XO &&
				board[i - 1][ix - 1] == XO &&
				board[i - 2][ix - 2] == XO &&
				board[i - 3][ix - 3] == XO)
			{
				win = 1;
			}


			if (board[i][ix] == XO &&
				board[i][ix - 1] == XO &&
				board[i][ix - 2] == XO &&
				board[i][ix - 3] == XO)
			{
				win = 1;
			}

			if (board[i][ix] == XO &&
				board[i - 1][ix] == XO &&
				board[i - 2][ix] == XO &&
				board[i - 3][ix] == XO)
			{
				win = 1;
			}

			if (board[i][ix] == XO &&
				board[i - 1][ix + 1] == XO &&
				board[i - 2][ix + 2] == XO &&
				board[i - 3][ix + 3] == XO)
			{
				win = 1;
			}

			if (board[i][ix] == XO &&
				board[i][ix + 1] == XO &&
				board[i][ix + 2] == XO &&
				board[i][ix + 3] == XO)
			{
				win = 1;
			}
		}

	}

	return win;
}

int FullBoard(char board[][10])
{
	int full;
	full = 0;
	for (int i = 1; i <= 7; ++i)
	{
		if (board[1][i] != '*')
			++full;
	}

	return full;
}

void PlayerWin(playerInfo activePlayer)
{
	cout << endl << activePlayer.playerName << "승리!" << endl;
}

int restart(char board[][10])
{
	int restart;

	cout << "게임을 다시 시작하겠습니까? 네(1) 아니오(2): ";
	cin >> restart;
	if (restart == 1)
	{
		for (int i = 1; i <= 6; i++)
		{
			for (int ix = 1; ix <= 7; ix++)
			{
				board[i][ix] = '*';
			}
		}
	}
	else
		cout << "Goodbye!" << endl;
	return restart;
}