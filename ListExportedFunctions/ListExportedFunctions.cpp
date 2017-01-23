#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <windowsx.h>

typedef FILE *PFILE;

#define APPLICATIONNAME "List exported functions\0"
#define CLASSNAME       "ListExportedFunctions\0"

#define IDC_FILENAME_EDIT   101
#define IDC_BUTTON          102
#define IDC_LISTVIEW        103
#define IDC_ADDRESS_EDIT    104

#define IDM_GET_NAME        201
#define IDM_GET_ADDRESS     202

HINSTANCE hInst;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void				ResizeControls(HWND, HWND, HWND, HWND, HWND);
void				CreateColumn(HWND);
void				OnButtonClick(HWND, HWND, HWND);
void				InsertItems(HWND, HWND, LPSTR);
void                ReadCString(PFILE, LPSTR);
DWORD               RvaToOffset(PIMAGE_SECTION_HEADER, WORD, DWORD);
void				OnRightClick(HWND);
void				OnGetAddressClick(HWND, HWND, HWND, HWND, int);
void                OnGetNameClick(HWND, HWND, HWND, HWND, int);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	INITCOMMONCONTROLSEX icex;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASSNAME;
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Stocke le handle d'instance dans la variable globale.

	hWnd = CreateWindow(CLASSNAME, APPLICATIONNAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 900, 600, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static HWND hwndFilenameEdit;
	static HWND hwndButton;
	static HWND hwndListView;
	static HWND hwndAddressEdit;

	static int iItem;

	switch (message)
	{
	case WM_CREATE:
		hwndFilenameEdit = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY | ES_LEFT,
			0, 0, 0, 0, hWnd, (HMENU)IDC_FILENAME_EDIT, hInst, NULL);

		hwndButton = CreateWindow("BUTTON", "Load...", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			0, 0, 0, 0, hWnd, (HMENU)IDC_BUTTON, hInst, NULL);

		hwndListView = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE | LVS_LIST,
			0, 0, 0, 0, hWnd, (HMENU)IDC_LISTVIEW, hInst, NULL);

		hwndAddressEdit = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY | ES_LEFT,
			0, 0, 0, 0, hWnd, (HMENU)IDC_ADDRESS_EDIT, hInst, NULL);

		CreateColumn(hwndListView);

		ResizeControls(hwndFilenameEdit, hwndButton, hwndListView, hwndAddressEdit, hWnd);
		break;
	case WM_SIZE:
		ResizeControls(hwndFilenameEdit, hwndButton, hwndListView, hwndAddressEdit, hWnd);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Analyse les sélections de menu :
		switch (wmId)
		{
		case IDC_BUTTON:
			OnButtonClick(hWnd, hwndFilenameEdit, hwndListView);
			break;
		case IDM_GET_NAME:
			OnGetNameClick(hWnd, hwndFilenameEdit, hwndListView, hwndAddressEdit, iItem);
			break;
		case IDM_GET_ADDRESS:
			OnGetAddressClick(hWnd, hwndFilenameEdit, hwndListView, hwndAddressEdit, iItem);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_RCLICK:
			LVHITTESTINFO lvhti;
			LPNMITEMACTIVATE lpnmia;

			lpnmia = (LPNMITEMACTIVATE)lParam;
			lvhti.pt = lpnmia->ptAction;

			if (ListView_SubItemHitTest(hwndListView, &lvhti) != -1)
			{
				iItem = lpnmia->iItem;
				OnRightClick(hWnd);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO : ajoutez ici le code de dessin...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

void ResizeControls(HWND hwndFilenameEdit, HWND hwndButton, HWND hwndListView, HWND hwndAddressEdit, HWND hWnd)
{
	RECT rc;

	GetClientRect(hWnd, &rc);

	MoveWindow(hwndFilenameEdit, 15, 15, rc.right - rc.left - (15 + 80 + 15 + 15), 25, TRUE);

	MoveWindow(hwndButton, rc.right - (15 + 80), 15, 80, 25, TRUE);

	MoveWindow(hwndListView, 15, 15 + 25 + 15, rc.right - rc.left - (15 + 15), rc.bottom - rc.top - (15 + 25 + 15 + 25 + 15 + 15), TRUE);

	MoveWindow(hwndAddressEdit, 15, rc.bottom - (15 + 25), rc.right - rc.left - (15 + 15), 25, TRUE);
}

void CreateColumn(HWND hwndListView)
{
	LVCOLUMN lvc;

	ListView_InsertColumn(hwndListView, 0, &lvc);
}

void OnButtonClick(HWND hWnd, HWND hwndFilenameEdit, HWND hwndListView)
{
	OPENFILENAME ofn;
	TCHAR szFile[1024];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = TEXT("Dynamic Link Library\0*.dll\0\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		Edit_SetText(hwndFilenameEdit, ofn.lpstrFile);
		return InsertItems(hWnd, hwndListView, ofn.lpstrFile);
	}
}

void InsertItems(HWND hWnd, HWND hwndListView, LPSTR lpstrFile)
{
	PFILE pfile = NULL;
	char name[1024];
	DWORD namePos = -1;
	DWORD offsetOfEntryExport = -1;
	DWORD offsetOfNames = -1;
	DWORD offsetOfNamePos = -1;

	IMAGE_DOS_HEADER        iDosHeader;
	IMAGE_NT_HEADERS        iNtHeaders;
	PIMAGE_SECTION_HEADER   piSectionHeader;
	IMAGE_EXPORT_DIRECTORY  iExportDir;

	LVITEM lvI;
	lvI.mask = LVIF_TEXT;

	ListView_DeleteAllItems(hwndListView);

	pfile = fopen(lpstrFile, "rb");

	if (pfile == NULL)
	{
		MessageBox(hWnd, "Impossible d'ouvrir le fichier.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	fread(&iDosHeader, sizeof(IMAGE_DOS_HEADER), 1, pfile);

	if (iDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox(hWnd, "Le fichier n'est pas valide.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	fseek(pfile, iDosHeader.e_lfanew, SEEK_SET);

	fread(&iNtHeaders, sizeof(IMAGE_NT_HEADERS), 1, pfile);

	if (iNtHeaders.Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox(hWnd, "Le fichier n'est pas valide.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	piSectionHeader = (PIMAGE_SECTION_HEADER)malloc(sizeof(IMAGE_SECTION_HEADER) * iNtHeaders.FileHeader.NumberOfSections);

	for (unsigned i = 0; i < iNtHeaders.FileHeader.NumberOfSections; i++)
	{
		fread(&piSectionHeader[i], sizeof(IMAGE_SECTION_HEADER), 1, pfile);
	}

	offsetOfEntryExport = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, iNtHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	if (offsetOfEntryExport == -1)
	{
		MessageBox(hWnd, "Impossible de trouver la table des fonctions exportées.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	fseek(pfile, offsetOfEntryExport, SEEK_SET);

	fread(&iExportDir, sizeof(IMAGE_EXPORT_DIRECTORY), 1, pfile);

	offsetOfNames = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, iExportDir.AddressOfNames);

	if (offsetOfNames == -1)
	{
		MessageBox(hWnd, "Impossible d'aller à l'adresse des noms.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	for (DWORD i = 0; i < iExportDir.NumberOfNames; i++)
	{
		fseek(pfile, offsetOfNames + i * sizeof(DWORD), SEEK_SET);
		fread(&namePos, sizeof(DWORD), 1, pfile);

		offsetOfNamePos = RvaToOffset(piSectionHeader, iNtHeaders.FileHeader.NumberOfSections, namePos);

		if (offsetOfNamePos == -1)
		{
			MessageBox(hWnd, "Impossible d'aller à l'adresse du nom.", "Erreur", MB_OK | MB_ICONERROR);
			return;
		}

		fseek(pfile, offsetOfNamePos, SEEK_SET);

		ReadCString(pfile, name);

		lvI.iItem = i;
		lvI.iSubItem = 0;
		lvI.pszText = name;
		ListView_InsertItem(hwndListView, &lvI);
	}

	SetFocus(hwndListView);
}

DWORD RvaToOffset(PIMAGE_SECTION_HEADER piSectionHeader, WORD numberOfSections, DWORD rva)
{
	for (WORD i = 0; i < numberOfSections; i++)
	{
		// La RVA est-elle dans cette section?
		if ((rva >= piSectionHeader[i].VirtualAddress) && (rva < piSectionHeader[i].VirtualAddress + piSectionHeader[i].SizeOfRawData))
		{
			rva -= piSectionHeader[i].VirtualAddress;
			rva += piSectionHeader[i].PointerToRawData;

			return rva;
		}
	}

	return -1;
}

void ReadCString(PFILE pfile, LPSTR name)
{
	DWORD n = 0;

	do
	{
		fread(name + n, sizeof(char), 1, pfile);
		n++;
	} while (name[n - 1] != 0 && n < 1023);

	name[n] = 0;
}

void OnRightClick(HWND hWnd)
{
	HMENU hMenu;
	POINT pt;

	hMenu = CreatePopupMenu();

	GetCursorPos(&pt);

	AppendMenu(hMenu, MF_STRING, IDM_GET_NAME, TEXT("Get name"));
	AppendMenu(hMenu, MF_STRING, IDM_GET_ADDRESS, TEXT("Get address"));

	TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hWnd, NULL);
}

void OnGetAddressClick(HWND hWnd, HWND hwndFilenameEdit, HWND hwndListView, HWND hwndAddressEdit, int iItem)
{
	CHAR szFile[1024];
	HMODULE hMod;
	DWORD dwAddress;
	CHAR szFunc[1024];
	CHAR szWorkBuff[12];

	Edit_GetText(hwndFilenameEdit, szFile, 1024);

	hMod = LoadLibrary(szFile);

	if (!hMod)
	{
		MessageBox(hWnd, "Impossible de charger la dll.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	ListView_GetItemText(hwndListView, iItem, 0, szFunc, 1024);

	dwAddress = (DWORD)GetProcAddress(hMod, szFunc);

	if (!dwAddress)
	{
		MessageBox(hWnd, "Impossible d'obtenir l'adresse.", "Erreur", MB_OK | MB_ICONERROR);
		return;
	}

	sprintf(szWorkBuff, "0x%X", dwAddress);

	Edit_SetText(hwndAddressEdit, szWorkBuff);
}

void OnGetNameClick(HWND hWnd, HWND hwndFilenameEdit, HWND hwndListView, HWND hwndAddressEdit, int iItem)
{
	CHAR szFunc[1024];

	ListView_GetItemText(hwndListView, iItem, 0, szFunc, 1024);

	Edit_SetText(hwndAddressEdit, szFunc);
}
