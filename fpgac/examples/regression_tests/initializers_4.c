volatile fpgac_output out:4;

int abc[16]:4 = {1, 2, 3, 5, 7, 11, 13, 17};

main()
{
	out = abc[3];
	out = abc[4];
	out = abc[6];
}



