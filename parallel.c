#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <strings.h>
#include <mpi.h>  

#define N 5000L
#define ND N*N/100

typedef struct {
    int i,j,v;
} tmd;

int A[N][N],B[N][N],C[N][N],C1[N][N],C2[N][N];
int jBD[N+1],VCcol[N],VBcol[N];
tmd AD[ND],BD[ND],CD[N*N];

long long Suma;

int cmp_fil(const void *pa, const void *pb)
{
    tmd * a = (tmd*)pa;
    tmd * b = (tmd*)pb;

    if (a->i > b->i) return(1);
    else if (a->i < b->i) return (-1);
    else return (a->j - b->j);
}

int cmp_col(const void *pa, const void *pb)
{
    tmd * a = (tmd*)pa;
    tmd * b = (tmd*)pb;

    if (a->j > b->j) return(1);
    else if (a->j < b->j) return (-1);
    else return (a->i - b->i);
}

int main(int na, char* args[])
{
    int i, j, k, neleC, neleC_aux;
    int el_meu_rank, p;
    
    MPI_Init(&na, &args);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &el_meu_rank);

    bzero(C, sizeof(int)*(N*N));
    bzero(C1, sizeof(int)*(N*N));
    bzero(C2, sizeof(int)*(N*N));
    
    for(k = 0; k < ND; k++)
    {
        AD[k].i = rand()%(N-1);
        AD[k].j = rand()%(N-1);
        AD[k].v = rand()%100 + 1;
        while (A[AD[k].i][AD[k].j]) 
        {
            if(AD[k].i < AD[k].j)
                AD[k].i = (AD[k].i + 1)%N;
            else 
                AD[k].j = (AD[k].j + 1)%N;
        }
        A[AD[k].i][AD[k].j] = AD[k].v;
    }

    qsort(AD, ND, sizeof(tmd), cmp_fil); // ordenat per files

    for(k = 0; k < ND; k++)
    {
        BD[k].i = rand()%(N-1);
        BD[k].j = rand()%(N-1);
        BD[k].v = rand()%100 + 1;
        while (B[BD[k].i][BD[k].j]) 
        {
            if(BD[k].i < BD[k].j)
                BD[k].i = (BD[k].i + 1)%N;
            else 
                BD[k].j = (BD[k].j + 1)%N;
        }
        B[BD[k].i][BD[k].j] = BD[k].v;
    }

    qsort(BD, ND, sizeof(tmd), cmp_col); // ordenat per columnes
    
    // calcul dels index de les columnes
    k = 0;
    for (j = 0; j < N+1; j++)
    {
      while (k < ND && j > BD[k].j) k++;
      jBD[j] = k;
    }

    int buff_size = N / p;
    int extra_size = N % p;
    int origin = 0, end = 0;
    int iterations[p], elements[p], offsets[p];

    for (i = 0; i < p; i++)
    {
        iterations[i] = buff_size;
        //if (i < extra_size) 
        //    iterations[i] = iterations[i] + 1;
        elements[i] = iterations[i] * N;
        offsets[i] = elements[i] * i;
    }
    if (extra_size != 0)
    {   
        iterations[p-1] += extra_size;
        elements[p-1] = iterations[p-1] * N;
    }
    origin = buff_size * el_meu_rank;
    end = origin + iterations[el_meu_rank];
    /*for (i = 0; i < el_meu_rank; i++)
        origin += iterations[i];
    end = origin + iterations[el_meu_rank];
    */

    //Matriu dispersa per matriu
    for(i = origin; i < end; i++)
        for (k = 0; k < ND; k++)
            C1[i][AD[k].i] += AD[k].v * B[AD[k].j][i];

    MPI_Gatherv(&C1[origin][0], elements[el_meu_rank], MPI_INT, &C1[0][0], elements, offsets, MPI_INT, 0, MPI_COMM_WORLD);  

    //Matriu dispersa per matriu dispersa
    for (j = 0; j < N; j++)
        VBcol[j] = 0;

    for(i = origin; i < end; i++)
    {
        // expandir Columna de B[*][i]
        for (k=jBD[i];k<jBD[i+1];k++)
                VBcol[BD[k].i] = BD[k].v;
        // Calcul de tota una columna de C
        for (k=0;k<ND;k++)
        {
            C2[i][AD[k].i] += AD[k].v * VBcol[AD[k].j];
        }
        // neteja vector de B[*][i]
        for (j=0;j<N;j++)
            VBcol[j] = 0;
    }

    MPI_Gatherv(&C2[origin][0], elements[el_meu_rank], MPI_INT, &C2[0][0], elements, offsets, MPI_INT, 0, MPI_COMM_WORLD);

    //Matriu dispersa per matriu dispersa -> dona matriu Dispersa
    neleC = 0;
    neleC_aux = 0;
    for (j = 0; j < N; j++)
        VBcol[j] = VCcol[j] = 0;

    for(i = origin; i < end; i++)
    {
        // expandir Columna de B[*][i]
        for (k = jBD[i]; k < jBD[i+1]; k++)
            VBcol[BD[k].i] = BD[k].v;
        // Calcul de tota una columna de C
        for (k = 0; k < ND; k++)
            VCcol[AD[k].i] += AD[k].v * VBcol[AD[k].j];
        for (j = 0; j < N; j++)
        {
            // neteja vector de B[*][i]
            VBcol[j] = 0;
            // Compressio de C
            if (VCcol[j])
            {
                CD[neleC_aux].i = j;
                CD[neleC_aux].j = i;
                CD[neleC_aux].v = VCcol[j];
                VCcol[j] = 0;
                neleC_aux++;
            }
        }
    }

    int neleCs[p];
    bzero(neleCs, p*sizeof(int));

    MPI_Reduce(&neleC_aux, &neleC, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    neleC_aux *= 3; // Struct de 3 ints
    
    MPI_Gather(&neleC_aux, 1, MPI_INT, neleCs, 1, MPI_INT, 0, MPI_COMM_WORLD);
    offsets[0] = 0;
    for(i = 1; i < p; i++)
    {
        offsets[i] = neleCs[i-1] + offsets[i-1];
    }

    MPI_Gatherv(CD, neleC_aux, MPI_INT, CD, neleCs, offsets, MPI_INT, 0, MPI_COMM_WORLD); 

    if (el_meu_rank == 0) 
    {
        // Comprovacio MD x M -> M i MD x MD -> M
        for (i = 0; i < N; i++)
            for(j = 0; j < N; j++)
                if (C2[j][i] != C1[j][i])
                    printf("Diferencies C1 i C2 pos %d,%d: %d != %d\n", i, j, C1[j][i], C2[j][i]);

        // Comprovacio MD X MD -> M i MD x MD -> MD
        Suma = 0;
        for(k = 0; k < neleC; k++)
        {
            Suma += CD[k].v;
            if (CD[k].v != C1[CD[k].j][CD[k].i])
                printf("Diferencies C1 i CD a i:%d,j:%d,v:%d, k:%d, vd:%d\n", CD[k].i, CD[k].j, C1[CD[k].i][CD[k].j], k, CD[k].v);
        }
         
        printf ("\nNumero elements de la matriu dispersa C %d\n", neleC);
        printf("Suma dels elements de C %lld \n", Suma);
    }
    MPI_Finalize();
    return (0);
}
