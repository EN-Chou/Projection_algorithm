#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
using namespace std;

//Macros
#define Re 100
#define delta_t 0.001
#define record_per 500 //Record per every record_per*delta_t sec
#define tol_vel pow(10, -6) //=delta_t
#define tol_p pow(10, -9)
#define N 81
#define PI 3.1415926

//Data exportatation 
string directory="./raw_data/Re_"+to_string(Re); 
string directory_moniter="./moniter"; 
ofstream raw_data(directory+"/flowfield.dat");
ofstream step_time(directory+"/stepstoconvergence_timestep.dat");

//Grid
double p[N+1][N+1]={1.0}, v[N+1][N+1]={0.0}, u[N+1][N+1]={0.0}; //current
double v_star[N+1][N+1]={0.0}, u_star[N+1][N+1]={0.0}; //u* and v*
double p_1[N+1][N+1]={1.0}, v_1[N+1][N+1]={0.0}, u_1[N+1][N+1]={0.0}; //previous
double p_c[N][N]={1.0}, v_c[N][N]={0.0}, u_c[N][N]={0.0}, vel_c[N][N]={0.0}; //collocated grid
double pred_p[N+1][N+1]={1.0};

double h=1.0/(N-1.0);
double res_vel=1.0, res_p=1.0, dev_p=1.0;
int iteration, timestep=0;

void setBC_v(), setBC_u(), setBC_p();
void cal_v_star(), cal_u_star(), cal_p(), cal_v(), cal_u();
bool velocity_not_converge(), pressure_not_converge();
void output(), collocate(), moniter();
double div_vel();

int main(){
    setBC_u();
    setBC_v();
    setBC_p();

    while(velocity_not_converge()||timestep<100){
        if(timestep%record_per==0){
            output();
            moniter();
        }

        //step 1
        cal_u_star();
        setBC_u();
        cal_v_star();
        setBC_v();
        //step 2
        cal_p();
        setBC_p();
        //step 3
        cal_u();
        setBC_u();
        cal_v();
        setBC_v();

        timestep++;
        moniter();
    }
    
    return 0;
}

void setBC_u(){
    for(int i=0; i<N; i++){
        /*Implement*/
        u[0][i]=-u[1][i]; u_star[0][i]=-u_star[1][i];
        u[N-1][i]=-u[N-2][i]; u_star[N-1][i]=-u_star[N-2][i];
        u[i][0]=-u[i][1]; u_star[i][0]=-u_star[i][1];
        u[i][N]=2.0-u[i][N-1]; u_star[i][N]=2.0-u_star[i][N-1];
    }
}
void setBC_v(){
    for(int i=0; i<N; i++){
        /*Implement*/
        v[0][i]=-v[1][i]; v_star[0][i]=-v_star[1][i];
        v[N][i]=-v[N-1][i]; v_star[N][i]=-v_star[N-1][i];
        v[i][0]=-v[i][1]; v_star[i][0]=-v_star[i][1];
        v[i][N-1]=-v[i][N-2]; v_star[i][N-1]=-v_star[i][N-2];
    }
}
void setBC_p(){
    /*Implement*/
    for(int i=0; i<N+1; i++){
        p[0][i]=p[1][i];
        p[N][i]=p[N-1][i];
        p[i][0]=p[i][1];
        p[i][N]=p[i][N-1];
    }
}

void cal_u_star(){
    /*Implement*/
    double v_p;
    double u_e, u_w, u_n, u_s;
    double C,D;
    for(int i=1; i<N-1; i++){
        for(int j=1; j<N; j++){
            v_p=(v[i][j]+v[i+1][j]+v[i][j-1]+v[i+1][j-1])*0.25;
            u_e=(u[i][j]+u[i+1][j])*0.5; u_w=(u[i-1][j]+u[i][j])*0.5; u_n=(u[i][j]+u[i][j+1])*0.5; u_s=(u[i][j]+u[i][j-1])*0.5;
            // C=....
            C=u[i][j]*(u_e-u_w)/h+v_p*(u_n-u_s)/h;
            // D=....
            D=1.0/Re*(u_e+u_w+u_n+u_s-4*u[i][j])/(h*h/4.0);
            u_star[i][j]=(D-C)*delta_t+u[i][j];
        }
    }
}
void cal_v_star(){
    /*Implement*/
    double u_p;
    double v_e, v_w, v_n, v_s;
    double C,D;
    for(int i=1; i<N; i++){
        for(int j=1; j<N-1; j++){
            u_p=(u[i-1][j]+u[i][j]+u[i-1][j+1]+u[i][j+1])*0.25;
            v_e=(v[i+1][j]+v[i][j])*0.5; v_w=(v[i-1][j]+v[i][j])*0.5; v_n=(v[i][j]+v[i][j+1])*0.5; v_s=(v[i][j]+v[i][j-1])*0.5;
            // C=v_p*(v_e-v_w)/h+u_p*(v_n-v_s)/h
            C=u_p*(v_e-v_w)/h+v[i][j]*(v_n-v_s)/h;
            // D=....
            D=1.0/Re*(v_e+v_w+v_n+v_s-4*v[i][j])/(h*h/4.0);
            v_star[i][j]=(D-C)*delta_t+v[i][j];
        }
    }
}

void cal_p(){
    int i,j;
    double term_poisson_left;
    double u_star_e, u_star_w, v_star_n, v_star_s;
    iteration=0;
        
    dev_p=0;
    for(i=0; i<N+1; i++){
        for(j=0; j<N+1; j++){
            pred_p[i][j]=p[i][j];
        }
    }
    
    do{
        iteration++, dev_p=0; double p_1=0;
        /*Implement*/
        for(i=1; i<N+1; i++){
            for(j=1; j<N+1; j++){
                p_1=p[i][j];
                u_star_e=u_star[i][j]; u_star_w=u_star[i-1][j]; v_star_n=v[i][j]; v_star_s=v[i][j-1];
                term_poisson_left=(u_star_e-u_star_w+v_star_n-v_star_s)/h/delta_t;
                p[i][j]=0.25*(p[i][j-1]+p[i][j+1]+p[i-1][j]+p[i+1][j]-term_poisson_left*h*h);
                dev_p+=fabs(p[i][j]-p_1);
            }
        }

        res_p=0;
        for(i=1;i<N+1; i++){
            for(j=1; j<N+1; j++){
                u_star_e=u_star[i][j]; u_star_w=u_star[i-1][j]; v_star_n=v[i][j]; v_star_s=v[i][j-1];
                term_poisson_left=(u_star_e-u_star_w+v_star_n-v_star_s)/h/delta_t;
                res_p+=fabs(p[i][j]-(0.25*(p[i][j-1]+p[i][j+1]+p[i-1][j]+p[i+1][j]-term_poisson_left*h*h)));
            }
        }
    }while(pressure_not_converge());
    
    for(i=0; i<N+1; i++){
        for(j=0; j<N+1; j++){
            dev_p+=fabs(pred_p[i][j]-p[i][j]);
        }
    }
    //[Time](timestep*delta_t) [Vel_deviation](res_vel) [Vel-divergence](div_vel()) 
    //[Iterations solving for P](iteration) 
    //[Prediction error of P](dev_p)
    cout<< timestep*delta_t <<" "<< iteration << "  "<< dev_p<<endl;

}
void cal_u(){
    res_vel=0;
    /*Implement*/
    for(int i=1; i<N-1;i++){
        for(int j=1; j<N; j++){
            u_1[i][j]=u[i][j];
            u[i][j]=u_star[i][j]-delta_t*(p[i+1][j]-p[i][j])/h;
            res_vel+=fabs(u[i][j]-u_1[i][j]);
        }
    }
    
}
void cal_v(){
    /*Implement*/
    for(int i=1; i<N;i++){
        for(int j=1; j<N-1; j++){
            v_1[i][j]=v[i][j];
            v[i][j]=v_star[i][j]-delta_t*(p[i][j+1]-p[i][j])/h;
            res_vel+=fabs(v[i][j]-v_1[i][j]);
        }
    }
}

bool pressure_not_converge(){
    if(res_p>tol_p)
        return true;
    else
        return false;
}
bool velocity_not_converge(){
    if(res_vel>tol_vel)
        return true;
    else
        return false;
}
double div_vel(){
    int i,j;
    double div=0.0;
    for(i=1; i<N-1; i++){
        for(j=1; j<N-1; j++){
            div+=fabs((u[i][j]-u[i-1][j])+(v[i][j]-v[i][j-1]));
        }
    }
    return div;
}
void collocate(){
    for(int i=0; i<N; i++){
        for(int j=0; j<N; j++){
            v_c[i][j]=0.5*(v[i][j]+v[i+1][j]);
            u_c[i][j]=0.5*(u[i][j]+u[i][j+1]);
            p_c[i][j]=0.25*(p[i][j]+p[i][j+1]+p[i+1][j]+p[i+1][j+1]);

        }
    }
    return;
}

void output(){ //Tecplot format
    if(timestep==0){
        raw_data<<"VARIABLES=\"x\", \"y\", \"time\", \"u\", \"v\", \"p\""<<endl;
        raw_data<<"ZONE T=\"1\""<<endl;
        raw_data<<"F=POINT"<<endl;
        raw_data<<"I=81,J=81,K=(define)"<<endl;
        step_time<<"step iteration"<<endl;
    }
    else{
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                raw_data<<1-i*h<<" "<<j*h<<"    "<< timestep*delta_t<<" "<<v_c[i][j]<<"   "<<u_c[i][j]<<"   "<<p_c[i][j]<<endl; 
            }
        }
    }
    step_time<< timestep << "   " << iteration <<endl;

    
    
}

void moniter(){
    collocate();

	ofstream moniter_u(directory_moniter+"/u.dat");
	ofstream moniter_v(directory_moniter+"/v.dat");
	ofstream moniter_p(directory_moniter+"/p.dat");
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			moniter_u << u_c[i][j] << ",";
			moniter_v << v_c[i][j] << ",";
			moniter_p << p_c[i][j] << ",";
		}
		moniter_u << endl;
		moniter_v << endl;
		moniter_p << endl;
	}
	moniter_u.close();
	moniter_v.close();
	moniter_p.close();
    return;
}
