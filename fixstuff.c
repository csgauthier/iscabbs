#include "defs.h"

int  usernumbers[1400];
char usernames[1400][22];

int
num_compare(const void *_a, const void *_b) 
{
 	int a = *(int*) _a;
	int b = *(int*) _b;
     if (usernumbers[a] < usernumbers[b])
	  return -1;
     if (usernumbers[a] == usernumbers[b])
	  return 0;
     return 1;
}

int
name_compare(const void *_a, const void *_b)
{
        int a = *(int*) _a;
        int b = *(int*) _b;
	int ret = (strncmp(usernames[a], usernames[b], 20));
	return ret;
}

int main() {
     
     struct userdata master;
     struct user users[1400];

     FILE *f = fopen(USERDATA, "r");
//     printf("f is %d\n", f); fflush(stdout);
     int t = fread(&master, sizeof(master), 1, f);
//     printf("t is %d\n", t);
     t = fread(users, sizeof(struct user), 1400, f);
//     printf("t is %d\n", t);
     fclose(f);
     
     int i;
     int j = 800;

     memset(usernames, 0, sizeof(usernames));
     memset(usernumbers,0,sizeof(usernumbers));


// populate record list
     for (i = 0; i < j; i++) {
          struct user* u = users+i;
          strcpy(usernames[i], u->name);
 	  usernumbers[i] = u->usernum;
     }


//     for (i = 0; i < 700; i++)
//         printf("user %3d is %s\n", i, usernames[i]);

        int which = master.which;
        int totalusers = master.totalusers[which];

#if 0 // list before we sort
        for ( i = 0; i < totalusers; i++) {
          printf("user %3d is number %3d (%-18s) and name %3d (%-18s)\n",
                i, 
		master.num[which][i], usernames[master.num[which][i]],
                master.name[which][i],usernames[master.name[which][i]]);
        }
#endif

// sort the lists properly
	qsort(&(master.num[which]),  totalusers,
	      sizeof(int), num_compare);
        qsort(&(master.name[which]), totalusers, 
	      sizeof(int), name_compare);

// now unique the lists

#if 0 // list after sorting, before we mess with things
        for ( i = 0; i < totalusers; i++) {
          printf("user %3d is number %3d (%-18s) and name %3d (%-18s)\n",
                i, 
		master.num[which][i], usernames[master.num[which][i]],
                master.name[which][i],usernames[master.name[which][i]]);
        }
#endif



// we first do unique by username, keeping track of which 
// records are duplicate.  We eliminate those during the
// pass through the usernumber range.


        int duplicates[30];
	int duplicatecount = 0;

	int prev = -1;
	int *array = (master.name[which]);
	int current = -1;
        for ( i = 0; i < totalusers; i++) {
            if (prev > -1 && 
		strcmp(usernames[prev],usernames[array[i]])==0) {
                   printf("duplicate entry: %3i %3i (at %3i) %s!\n",
			  prev, array[i], i, usernames[prev]);
		   duplicates[duplicatecount] = prev;
		   printf("prev[%d] is %d...\n", duplicatecount, prev);
		   duplicatecount++;
		   prev = -1; // don't take out triples     
       } else {
		   current++;
  	    }
#if 0
	// weird special case: kill user "Violently Happy,"
	// at user record 312
	    if (prev == 312)
		    current--;
#endif
            prev = array[i];
	    array[current] = array[i];
 
       }

// usernum pass
	prev = -1;
        int passtwokill = 0;
        array = (master.num[which]);
	current = -1;
	for ( i = 0; i < totalusers; i++) {
	    int killthisuser = 0;
	    int k;
	    for (k = 0; k < duplicatecount; k++) {
		if (duplicates[k] == prev) {
		    killthisuser = 1;
	   	    // wipe this entry out of the list:
		    duplicates[k] = -1;
		    break; // k
        	}
	    }	
           if (killthisuser) {
        	    printf("duplicate entry %d???: %3i %3i (at %3i) %s!\n",
			   killthisuser,
                           prev, array[i], i, usernames[prev]);
                    if (!killthisuser) passtwokill++; 
            } else {
            	current++;
            }
	    prev = array[i];
            array[current] = array[i];
	}
//      duplicatecount++;
        duplicatecount += passtwokill;
// drop the user count by deleted users, plus 0 for "Happy"
	totalusers -= (duplicatecount  ); 
	master.totalusers[which] -= (duplicatecount );
 printf("\n got rid of %d users, now have %d xxx \n",
     duplicatecount, totalusers);
#if 0 // show list after
        for ( i = 0; i < totalusers; i++) {
          printf("Xuser %3d is number %3d (%-18s) and name %3d (%-18s)\n",
                i,
                master.num[which][i], usernames[master.num[which][i]],
                master.name[which][i],usernames[master.name[which][i]]);
        }
#endif
// write stuff back to the user file.
// Change "1" to "0" below to make the program harmless.
#if 1

	FILE *g = fopen(USERDATA, "w");
	t = fwrite(&master, sizeof(master), 1, g);
//	printf("t is %d\n", t);
	t = fwrite(users, sizeof(struct user), 1400, g);
//	printf("t is %d\n", t);
	fclose(g);
#endif
     return 0;
}


