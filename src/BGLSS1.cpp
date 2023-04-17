#include<RcppArmadillo.h>
#include<stdio.h>
#include<vector>
#include"BVCUtilities.h"
// [[Rcpp::depends(RcppArmadillo)]]
using namespace Rcpp;
using namespace arma;
using namespace std;
//using namespace R;

// [[Rcpp::export()]]

Rcpp::List BGLSS_1 (arma::mat y, arma:: mat e, arma:: mat C, arma::mat g, arma:: mat w, arma::vec z,int maxSteps, int n, int k,arma::vec hatBeta, arma:: vec hatEta, arma::vec hatAlpha, arma::vec hatAta,arma::vec hatInvTauSq1, arma::vec hatInvTauSq2,double hatPiEta, double hatPiBeta, arma::vec invSigAlpha0, double hatLambdaSqStar1, double hatLambdaSqStar2, double hatSigmaSq, double hatPhiSq,double aStar, double bStar, double alpha, double gamma,double alpha1,double gamma1, double mu0, double nu0)
{
  unsigned int q = e.n_cols-1,m = g.n_cols,p = w.n_cols;
  arma::mat gsAlpha(maxSteps, q+3),
  gsBeta(maxSteps,m),
  gseta(maxSteps,p),
  gsAta(maxSteps,n),
  gsInvTauSq1(maxSteps,m),
  gsInvTauSq2(maxSteps, m);


  arma::vec
    gsLambdaStar1(maxSteps),
    gsLambdaStar2(maxSteps),
    gsSigmaSq(maxSteps),
    gsPhiSq(maxSteps),
    gsPiBeta(maxSteps),
    gsPiEta(maxSteps);



  arma::mat mat0,mat1,mat2,ei,gi,wi;


  double meanAlpha;
  double varAlpha;

  double meanb;
  double varb;


  for (int t = 0; t < maxSteps; t++) {

    mat0 = arma::repelem(e,k,1);
    mat1 = arma::repelem(g,k,1);
    mat2 = arma::repelem(w,k,1);


    // alpha|
    for(unsigned int j=0;j<q+3;j++){
      arma::vec res1, res11;
      double A0;
      A0 =0;
      double B0;
      B0=0;
      for(int i=0;i<n;i++){
        ei = mat0.rows((i*k),(i*k+k-1));
        ei.insert_cols(2, C);
        gi = mat1.rows((i*k),(i*k+k-1));
        wi = mat2.rows((i*k),(i*k+k-1));

        A0 = A0+arma::as_scalar(ei.col(j).t()*ei.col(j))/hatSigmaSq;
        res1 = y.row(i).t()-ei*hatAlpha-gi*hatBeta-wi*hatEta-z*hatAta(i);
        res11 = res1+ei.col(j)*hatAlpha(j);
        B0 = B0+arma::as_scalar(ei.col(j).t()*res11)/hatSigmaSq;

      }



      meanAlpha = arma::as_scalar(B0/(invSigAlpha0(j)+A0));


      varAlpha = arma::as_scalar(1/(invSigAlpha0(j)+A0));

      hatAlpha(j) = R::rnorm(meanAlpha,sqrt(varAlpha));

    }


    gsAlpha.row(t) = hatAlpha.t();

    // ata|
    arma::vec res;
    for(int i=0;i<n;i++){
      ei = mat0.rows((i*k),(i*k+k-1));
      ei.insert_cols(2, C);
      gi = mat1.rows((i*k),(i*k+k-1));
      wi = mat2.rows((i*k),(i*k+k-1));
      double tzz;
      tzz = arma::accu(arma::square(z));
      double A;
      A = tzz/hatSigmaSq;
      res = y.row(i).t()-ei*hatAlpha-gi*hatBeta-wi*hatEta;
      double B;
      B = arma::as_scalar(z.t()*res)/hatSigmaSq;
      double varAta;
      varAta = 1/(A+1/hatPhiSq);
      double meanAta;
      meanAta = arma::as_scalar(B/(A+1/hatPhiSq));
      hatAta(i) = R::rnorm(meanAta, std::sqrt(varAta));
    }


    gsAta.row(t) = hatAta.t();


    // Beta|

    double l1;
    double u;

    double tempb;
    for(unsigned int j=0;j<m;j++){
      arma::vec res2, res22;
      double A1;
      A1=0;
      double B1;
      B1=0;

      for(int i=0;i<n;i++){
        ei = mat0.rows((i*k),(i*k+k-1));
        ei.insert_cols(2, C);
        gi = mat1.rows((i*k),(i*k+k-1));
        wi = mat2.rows((i*k),(i*k+k-1));
        arma::vec tggDiag = sum(square(gi),0).t();
        A1 = A1+tggDiag(j);
        res2 = y.row(i).t()-ei*hatAlpha-gi*hatBeta-wi*hatEta-z*hatAta(i);
        res22 = res2+gi.col(j)*hatBeta(j);
        B1 = B1+ arma::as_scalar(gi.col(j).t()*res22);

      }
      meanb = arma::as_scalar(B1/(hatInvTauSq1(j)+A1));

      varb = arma::as_scalar(hatSigmaSq/(hatInvTauSq1(j)+A1));

      tempb = arma::as_scalar(sqrt(hatInvTauSq1(j)/(hatInvTauSq1(j)+A1))*exp(pow(B1,2)/(2*hatSigmaSq*(hatInvTauSq1(j)+A1))));

      l1 = hatPiBeta/(hatPiBeta+(1-hatPiBeta)*tempb);
      u = R::runif(0,1);
      if(u<l1){
        hatBeta(j)=0;
      }
      else{
        hatBeta(j)=R::rnorm(meanb,sqrt(varb));
      }

    }

    gsBeta.row(t) = hatBeta.t();
    // eta|

    arma::vec meane;
    arma::mat varcove(q,q);
    double tempe,l2;
    arma:: mat Tau(q,q);
    Tau = Tau.eye();


    for(unsigned int j=0;j<m;j++){
      arma::mat A2(q,q);
      A2 = A2.zeros();
      arma:: vec B2;
      B2 = zeros<vec>(q);

      arma::vec res3, res33;

      for(int i=0;i<n;i++){
        ei = mat0.rows((i*k),(i*k+k-1));
        ei.insert_cols(2, C);
        gi = mat1.rows((i*k),(i*k+k-1));
        wi = mat2.rows((i*k),(i*k+k-1));
        A2 = A2+wi.cols((j*q),(j*q+q-1)).t()*wi.cols((j*q),(j*q+q-1));
        res3 = y.row(i).t()-ei*hatAlpha-gi*hatBeta-wi*hatEta-z*hatAta(i);
        res33 = res3+wi.cols((j*q),(j*q+q-1))*hatEta.subvec((j*q),(j*q+q-1));
        B2 = B2+wi.cols((j*q),(j*q+q-1)).t()*res33;

      }

      meane = arma::inv(hatInvTauSq2(j)*Tau+A2)*B2;

      varcove = hatSigmaSq*arma::inv(hatInvTauSq2(j)*Tau+A2);
      tempe = arma::as_scalar((std::pow(hatInvTauSq2(j),q/2)*sqrt(arma::det(arma::inv(hatInvTauSq2(j)*Tau+A2)))*exp(B2.t()*arma::inv(hatInvTauSq2(j)*Tau+A2)*B2))/(2*hatSigmaSq));
      l2 = hatPiEta/(hatPiEta+(1-hatPiEta)*tempe);
      u = R::runif(0,1);
      if(u<l2){

        hatEta.subvec((j*q),(j*q+q-1)).zeros();
      }
      else{


        hatEta.subvec((j*q),(j*q+q-1)) = mvrnormCpp(meane,varcove);
      }

    }

    gseta.row(t) = hatEta.t();

    // invTAUsq.star1|

    double lInvTauSq1;
    lInvTauSq1   = hatLambdaSqStar1;
    arma::vec muInvTauSq1;
    muInvTauSq1 = arma::sqrt(hatLambdaSqStar1 * hatSigmaSq / arma::square(hatBeta));
    for(unsigned int j = 0; j<m; j++){
      if(hatBeta(j)==0){
        hatInvTauSq1(j) = 1/R::rgamma(1,2/lInvTauSq1);
      }else{
        hatInvTauSq1(j) = rinvgaussian(muInvTauSq1(j), lInvTauSq1);
      }

    }

    gsInvTauSq1.row(t) = hatInvTauSq1.t();

    // invTAUsq.star2|
    double lInvTauSq2;
    arma:: mat matRStar;
    arma:: vec tRsRs, muInvTauSq2;
    lInvTauSq2 = q*hatLambdaSqStar2;
    matRStar = arma::reshape(hatEta, q, m);
    tRsRs = arma::sum(arma::square(matRStar), 0).t();
    muInvTauSq2 = arma::sqrt(q * hatLambdaSqStar2 * hatSigmaSq / tRsRs);
    for(unsigned int j = 0; j<m; j++){
      if(tRsRs(j) == 0){
        hatInvTauSq2(j) = 1/R::rgamma((q+1)/2, 2/lInvTauSq2);
      }else{
        hatInvTauSq2(j) = rinvgaussian(muInvTauSq2(j), lInvTauSq2);
      }
    }
    gsInvTauSq2.row(t) = hatInvTauSq2.t();


    // sigma.sq|
    arma:: vec repInvTauSq;
    double shapeSig, rateSig;
    shapeSig = alpha + n*k/2 + arma::accu(hatBeta!=0)/2+ q*arma::accu(tRsRs!=0)/2;
    double ress;
    ress=0;
    for(int i=0;i<n;i++){
      ei = mat0.rows((i*k),(i*k+k-1));
      ei.insert_cols(2, C);
      gi = mat1.rows((i*k),(i*k+k-1));
      wi = mat2.rows((i*k),(i*k+k-1));
      arma::vec res4;
      res4 = y.row(i).t()-ei*hatAlpha-gi*hatBeta-wi*hatEta-z*hatAta(i);
      ress = ress+arma::accu(arma::square(res4));
    }
    repInvTauSq = arma::vectorise(arma::repelem(hatInvTauSq2.t(), q, 1), 0);
    rateSig = gamma + 0.5*(ress +arma::accu(square(hatBeta)%hatInvTauSq1)+ arma::accu(square(hatEta) % repInvTauSq));

    hatSigmaSq = 1/R::rgamma(shapeSig, 1/rateSig);
    gsSigmaSq(t) = hatSigmaSq;


    // phi.sq|
    double shapePhi, ratePhi;
    shapePhi = alpha1 + n/2;

    ratePhi = gamma1 + 0.5*(arma::accu(square(hatAta)));

    hatPhiSq = 1/R::rgamma(shapePhi, 1/ratePhi);
    gsPhiSq(t) = hatPhiSq;

    // lambda.star1|
    double shapeS1;
    shapeS1= aStar + m;
    double rateS1;
    rateS1= bStar + arma::accu(1/hatInvTauSq1)/2;
    hatLambdaSqStar1 = R::rgamma(shapeS1, 1/rateS1);
    gsLambdaStar1(t) = hatLambdaSqStar1;

    // lambda.star2|
    double shapeS2;
    shapeS2= aStar + m*(q+1)/2;
    double rateS2;
    rateS2= bStar + q*arma::accu(1/hatInvTauSq2)/2;
    hatLambdaSqStar2 = R::rgamma(shapeS2, 1/rateS2);
    gsLambdaStar2(t) = hatLambdaSqStar2;

    //pi.beta|
    double shape1_b;
    shape1_b= mu0 +arma::accu(hatBeta==0);
    double shape2_b;
    shape2_b= nu0 + arma::accu(hatBeta!=0);
    hatPiBeta = R::rbeta(shape1_b, shape2_b);
    gsPiBeta(t) = hatPiBeta;

    // pi.eta|
    double shape1_e;
    shape1_e= mu0 + arma::accu(tRsRs == 0);
    double shape2_e;
    shape2_e= nu0 + arma::accu(tRsRs != 0);
    hatPiEta = R::rbeta(shape1_e, shape2_e);
    gsPiEta(t) = hatPiEta;



  }

  return Rcpp::List::create(
    Rcpp::Named("GS.alpha") = gsAlpha,
    Rcpp::Named("GS.beta") = gsBeta,
    Rcpp::Named("GS.eta") = gseta,
    Rcpp::Named("GS.ata") = gsAta,
    Rcpp::Named("GS.invTAUsq1") = gsInvTauSq1,
    Rcpp::Named("GS.invTAUsq2") = gsInvTauSq2,
    Rcpp::Named("GS.lambda.sq1") = gsLambdaStar1,
    Rcpp::Named("GS.lambda.sq2") = gsLambdaStar2,
    Rcpp::Named("GS.sigma.sq") = gsSigmaSq,
    Rcpp::Named("GS.phi.sq") = gsPhiSq,
    Rcpp::Named("GS.Pi.Beta") = gsPiBeta,
    Rcpp::Named("GS.Pi.Eta") = gsPiEta)
    ;

}