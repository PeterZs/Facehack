//
//  ofTest.h
//
//  oFアプリでのテストクラス
//
//  Copyright (c) 2016年 Takahiro Kosaka. All rights reserved.
//  Created by Takahiro Kosaka on 2016/04/05.
//
//  This Source Code Form is subject to the terms of the Mozilla
//  Public License v. 2.0. If a copy of the MPL was not distributed
//  with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ofTest.h"
#include "System/Math/KSMath.h"
#include "System/Util/KSUtil.h"
#include "ofxTimeMeasurements.h"

using namespace std;
using namespace Kosakasakas;

ofTest::ofTest()
{}

ofTest::~ofTest()
{}

bool    ofTest::Initialize()
{
    return true;
}

bool    ofTest::DoTest()
{
    // 最適化テスト
    
    // 例題No.1
    {
        // ==================================
        // y=2x+1 を解として y=ax+b を最適化する
        // ==================================
        
        // データセットを登録。ただし、一つだけ5%の誤差を含んでいる
        KSMatrixXf  data(2, 5);
        data << 0.01, 0.04, 0.08, 0.12 * 1.005, 0.16,
                1.02, 1.08, 1.16, 1.24, 1.32;
        
        // オプティマイザの宣言
        KSDenseOptimizer  optimizer;
        
        // 残差関数
        KSFunction  residual    = [&optimizer](const KSVectorXf &x)->KSVectorXf
        {
            const KSMatrixXf& data = optimizer.GetDataMat();
            KSMatrixXf r(data.cols(), 1);
            
            for(int i=0, n=r.rows(); i<n; ++i)
            {
                // r = y-(ax+b)
                r(i)    = data(1,i) - (x(0) * data(0, i) + x(1));
            }
            
            return r;
        };
        
        // 残差のヤコビアン
        KSFunction jacobian     = [&optimizer](const KSVectorXf &x)->KSMatrixXf
        {
            const KSMatrixXf& data = optimizer.GetDataMat();
            KSMatrixXf d(data.cols(), x.rows());
            
            for(int i=0,n=d.rows(); i<n; ++i)
            {
                // d0 = -x
                // d1 = -1
                d(i, 0)         = -data(0,i);
                d(i, 1)         = -1;
            }
            
            return d;
        };
        
        // パラメータ行列の初期値を設定
        KSMatrixXf param0(2,1);
        param0 << 5.0, 5.0;
        
        // オプティマイザの初期化
        optimizer.Initialize(residual, jacobian, param0, data);
        
        // 計算ステップ5回
        int numStep = 5;
        
        // 計算開始(通常計算)
        TS_START("optimization exmple 1-1");
        for (int i = 0; i < numStep; ++i)
        {
            if (!optimizer.DoGaussNewtonStep())
            {
                ofLog(OF_LOG_ERROR, "ガウス-ニュートン計算ステップに失敗しました。");
                return false;
            }
        }
        TS_STOP("optimization exmple 1-1");
        
        // 解の確認
        ofLog(OF_LOG_NOTICE,
              "ex1-1: param0: %lf, param1: %lf",
              optimizer.GetParamMat()(0),
              optimizer.GetParamMat()(1));
        
        // パラメータ行列の初期値を再設定
        KSMatrixXf param1(2,1);
        param1 << 5.0, 5.0;
        optimizer.SetParamMat(param1);
        
        // 計算開始(IRLS計算)
        TS_START("optimization exmple 1-2");
        for (int i = 0; i < numStep; ++i)
        {
            if (!optimizer.DoGaussNewtonStepIRLS())
            {
                ofLog(OF_LOG_ERROR, "ガウス-ニュートン計算ステップに失敗しました。");
                return false;
            }
        }
        TS_STOP("optimization exmple 1-2");
        
        // 解の確認
        ofLog(OF_LOG_NOTICE,
              "ex1-2: param0: %lf, param1: %lf",
              optimizer.GetParamMat()(0),
              optimizer.GetParamMat()(1));
        
        // ================================
        // 結果:
        // [notice ] ex1-1: param0: 1.996821, param1: 1.000021
        // [notice ] ex1-2: param0: 2.000000, param1: 1.000000
        //
        // IRLSの方が誤差を含むデータに対して高精度な解が得られる。
        // ただし、0.1msほど計算が遅い。
        // ================================

    }
    
    // 例題No.2
    {
        // ==================================
        // wikiの例題を解く
        // https://en.wikipedia.org/wiki/Gauss%E2%80%93Newton_algorithm
        // ==================================
        
        // データセットを登録
        KSMatrixXf  data(2, 7);
        data <<  0.038, 0.194, 0.425, 0.626,  1.253,  2.500,  3.740,
        0.050, 0.127, 0.094, 0.2122, 0.2729, 0.2665, 0.3317;
        
        // オプティマイザの宣言
        KSDenseOptimizer  optimizer;
        
        // 残差関数
        KSFunction  residual    = [&optimizer](const KSVectorXf &x)->KSVectorXf
        {
            const KSMatrixXf& data = optimizer.GetDataMat();
            KSVectorXf y(data.cols());
            
            for(int i=0, n=y.rows(); i<n; ++i)
            {
                y(i)    = data(1,i) - (x(0) * data(0, i)) / (x(1) + data(0,i));
            }
            return y;
        };
        
        // 残差のヤコビアン
        KSFunction jacobian     = [&optimizer](const KSVectorXf &x)->KSMatrixXf
        {
            const KSMatrixXf& data = optimizer.GetDataMat();
            KSMatrixXf d(data.cols(), x.rows());
            
            for(int i=0,n=d.rows(); i<n; ++i)
            {
                double denom    = (x(1) + data(0,i)) * (x(1) + data(0,i));
                d(i, 0)         = -data(0,i) / (x(1) + data(0,i));
                d(i, 1)         = (x(1) * data(0, i)) / denom;
            }
            return d;
        };
        
        // 正解値マトリックスの初期値を設定
        KSMatrixXf param(2,1);
        param << 0.9, 0.2;
        
        // オプティマイザの初期化
        optimizer.Initialize(residual, jacobian, param, data);
        
        ofASSERT((optimizer.GetSquaredResidualsSum() - 1.445) < 0.01, "残差平方和の初期値が正解と異なります。");
        
        std::vector<double> srsLog;
        
        TS_START("optimization exmple 2");
        for (int i = 0; i < 5; ++i)
        {
            ofASSERT(optimizer.DoGaussNewtonStep(), "ガウス-ニュートン計算ステップに失敗しました。");
            srsLog.push_back(optimizer.GetSquaredResidualsSum());
        }
        TS_STOP("optimization exmple 2");
        
        //各ステップでの残差平方和
        ofLog(OF_LOG_NOTICE,
              "step0:%lf, step1:%lf, step2:%lf, step3:%lf, step4:%lf",
              srsLog[0], srsLog[1], srsLog[2], srsLog[3], srsLog[4]);
        
        ofASSERT((optimizer.GetSquaredResidualsSum() - 0.00784) < 0.01, "残差平方和の収束値が正解と異なります。");
        
        ofASSERT(fabs(optimizer.GetParamMat()(0) - 0.362) < 0.01, "パラメータ推定結果が異なります。");
        ofASSERT(fabs(optimizer.GetParamMat()(1) - 0.556) < 0.01, "パラメータ推定結果が異なります。");
        
        // ================================
        // 結果:
        // [notice ] step0:0.008561, step1:0.007904, step2:0.007855, step3:0.007846, step4:0.007844
        //
        // 残差平方和はステップごとに縮まっていて、
        // wikiの正解値である0.00784と同値(0.007844)が得られる。
        // ================================
    }
    
    // 例題No.3
    {
        // ==================================
        // 例題No.2と同じ問題をSparse行列を使い、
        // 前処理付き共役勾配法を使って解く
        // ==================================
    
        // データセットを登録
        KSMatrixSparsef  data(2, 7);
        data.insert(0, 0) = 0.038;
        data.insert(0, 1) = 0.194;
        data.insert(0, 2) = 0.425;
        data.insert(0, 3) = 0.626;
        data.insert(0, 4) = 1.253;
        data.insert(0, 5) = 2.500;
        data.insert(0, 6) = 3.740;
        data.insert(1, 0) = 0.050;
        data.insert(1, 1) = 0.127;
        data.insert(1, 2) = 0.094;
        data.insert(1, 3) = 0.2122;
        data.insert(1, 4) = 0.2729;
        data.insert(1, 5) = 0.2665;
        data.insert(1, 6) = 0.3317;
        
        // オプティマイザの宣言
        KSSparseOptimizer  optimizer;
        
        // ソルバを前処理付き共役勾配法(PGC)に変更
        optimizer.SwitchNormalEquationSolver(NESolverType::PCG);
        
        // PGCの試行回数のセット
        optimizer.SetMaxIterations(4);
        
        // 残差関数
        KSFunctionSparse  residual    = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef y(data.cols(), 1);
            
            for(int i=0, n=y.rows(); i<n; ++i)
            {
                y.coeffRef(i, 0) = data.coeff(1, i) - (x.coeff(0, 0) * data.coeff(0, i)) / (x.coeff(1, 0) + data.coeff(0,i));
            }
            return y;
        };
        
        // 残差のヤコビアン
        KSFunctionSparse jacobian     = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef d(data.cols(), x.rows());
            
            for(int i=0,n=d.rows(); i<n; ++i)
            {
                double denom    = (x.coeff(1, 0) + data.coeff(0,i)) * (x.coeff(1, 0) + data.coeff(0,i));
                d.coeffRef(i, 0)         = -data.coeff(0,i) / (x.coeff(1, 0) + data.coeff(0,i));
                d.coeffRef(i, 1)         = (x.coeff(1, 0) * data.coeff(0, i)) / denom;
            }
            return d;
        };
        
        // 正解値マトリックスの初期値を設定
        KSMatrixSparsef param(2,1);
        param.insert(0, 0) = 0.9;
        param.insert(1, 0) = 0.2;
        
        // オプティマイザの初期化
        optimizer.Initialize(residual, jacobian, param, data);
        
        ofASSERT((optimizer.GetSquaredResidualsSum() - 1.445) < 0.01, "残差平方和の初期値が正解と異なります。");
        
        std::vector<double> srsLog;
        
        TS_START("optimization exmple 3");
        for (int i = 0; i < 5; ++i)
        {
            ofASSERT(optimizer.DoGaussNewtonStep(), "ガウス-ニュートン計算ステップに失敗しました。");
            srsLog.push_back(optimizer.GetSquaredResidualsSum());
        }
        TS_STOP("optimization exmple 3");
        
        //各ステップでの残差平方和
        ofLog(OF_LOG_NOTICE,
              "step0:%lf, step1:%lf, step2:%lf, step3:%lf, step4:%lf",
              srsLog[0], srsLog[1], srsLog[2], srsLog[3], srsLog[4]);
        
        ofASSERT((optimizer.GetSquaredResidualsSum() - 0.00784) < 0.01, "残差平方和の収束値が正解と異なります。");
        
        ofASSERT(fabs(optimizer.GetParamMat().coeff(0, 0) - 0.362) < 0.01, "パラメータ推定結果が異なります。");
        ofASSERT(fabs(optimizer.GetParamMat().coeff(1, 0) - 0.556) < 0.01, "パラメータ推定結果が異なります。");
    }
    
    // 例題No.4
    {
        // 剛体変換のパラメータを最適化で解いてみる。
        // X軸の回転角をa, Y軸の回転角をb, Z軸の回転角をc、
        // 平行移動成分を(vx, vy, vz)とすると、未知数は6個
        // よってそれ以上の残差が与えられれば解けるはず
        
        // 正解値を適当に与える
        const int paramNum = 6;
        float anser[paramNum];
        anser[0] = ofDegToRad(20.0f); // X軸回転角
        anser[1] = ofDegToRad(30.0f); // Y軸回転角
        anser[2] = ofDegToRad(10.0f); // Z軸回転角
        anser[3] = 30.0f; // X軸平行移動
        anser[4] = 20.0f; // Y軸平行移動
        anser[5] = 10.0f;  // Z軸平行移動
        
        // X、Y、Z軸の順で回転する行列を作成
        ofMatrix4x4 m;
        m.makeRotationMatrix(ofRadToDeg(anser[0]), ofVec3f(1.0f, 0.0f, 0.0f),
                             ofRadToDeg(anser[1]), ofVec3f(0.0f, 1.0f, 0.0f),
                             ofRadToDeg(anser[2]), ofVec3f(0.0f, 0.0f, 1.0f));
        // 平行移動成分
        ofVec3f t = {anser[3], anser[4], anser[5]};
        
        // 適当に入力データサンプルを作る
        int sampleVecNum = 20;
        KSMatrixSparsef data(2, 3 * sampleVecNum);
        for (int i = 0; i < sampleVecNum; ++i)
        {
            ofVec3f v;
            v.x = ofRandom(-10.0f, 10.0f);
            v.y = ofRandom(-10.0f, 10.0f);
            v.z = ofRandom(-10.0f, 10.0f);
            
            ofVec3f a = ofMatrix4x4::transform3x3(m, v);
            a += t;
            
            data.insert(0, 3*i+0) = v.x;
            data.insert(0, 3*i+1) = v.y;
            data.insert(0, 3*i+2) = v.z;
            
            data.insert(1, 3*i+0) = a.x;
            data.insert(1, 3*i+1) = a.y;
            data.insert(1, 3*i+2) = a.z;
        }
        
        // オプティマイザの宣言
        KSSparseOptimizer  optimizer;
        
        // ソルバを前処理付き共役勾配法(PGC)に変更
        optimizer.SwitchNormalEquationSolver(NESolverType::PCG);
        
        // PGCの試行回数のセット
        optimizer.SetMaxIterations(4);
        
        // 残差関数
        KSFunctionSparse  residual    = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef d(data.cols(), 1);
            
            float a = x.coeff(0, 0); // X軸回転角
            float b = x.coeff(1, 0); // Y軸回転角
            float c = x.coeff(2, 0); // Z軸回転角
            
            KSMatrixSparsef mt(4,4);
            
             // X,Y,Z回転回転
            mt.coeffRef(0, 0) = cosf(b) * cosf(c);
            mt.coeffRef(0, 1) = sinf(a) * sinf(b) * cosf(c) - cosf(a) * sinf(c);
            mt.coeffRef(0, 2) = cosf(a) * sinf(b) * cosf(c) + sinf(a) * sinf(c);
            
            mt.coeffRef(1, 0) = cosf(b) * sinf(c);
            mt.coeffRef(1, 1) = sinf(a) * sinf(b) * sinf(c) + cosf(a) * cosf(c);
            mt.coeffRef(1, 2) = cosf(a) * sinf(b) * sinf(c) - sinf(a) * cosf(c);
            
            mt.coeffRef(2, 0) = -sinf(b);
            mt.coeffRef(2, 1) = sinf(a) * cosf(b);
            mt.coeffRef(2, 2) = cosf(a) * cosf(b);
            
            /*
            // X回転
            mt.coeffRef(0, 0) = 1.0f;
            mt.coeffRef(0, 1) = 0.0f;
            mt.coeffRef(0, 2) = 0.0f;
            
            mt.coeffRef(1, 0) = 0.0f;
            mt.coeffRef(1, 1) = cosf(a);
            mt.coeffRef(1, 2) = -sinf(a);
            
            mt.coeffRef(2, 0) = 0.0f;
            mt.coeffRef(2, 1) = sinf(a);
            mt.coeffRef(2, 2) = cosf(a);
            */
            
            /*
            // Y回転
            mt.coeffRef(0, 0) = cosf(b);
            mt.coeffRef(0, 1) = 0.0f;
            mt.coeffRef(0, 2) = sinf(b);
            
            mt.coeffRef(1, 0) = 0.0f;
            mt.coeffRef(1, 1) = 1.0f;
            mt.coeffRef(1, 2) = 0.0f;
            
            mt.coeffRef(2, 0) = -sinf(b);
            mt.coeffRef(2, 1) = 0.0f;
            mt.coeffRef(2, 2) = cosf(b);
             */
            
            /*
             // Z回転
             mt.coeffRef(0, 0) = cosf(c);
             mt.coeffRef(0, 1) = -sinf(c);
             mt.coeffRef(0, 2) = 0.0f;
             
             mt.coeffRef(1, 0) = sinf(c);
             mt.coeffRef(1, 1) = cosf(c);
             mt.coeffRef(1, 2) = 0.0f;
             
             mt.coeffRef(2, 0) = 0.0;
             mt.coeffRef(2, 1) = 0.0f;
             mt.coeffRef(2, 2) = 1.0f;
             */
            
            // 平行移動成分
            mt.coeffRef(3, 0) = x.coeff(3, 0);
            mt.coeffRef(3, 1) = x.coeff(4, 0);
            mt.coeffRef(3, 2) = x.coeff(5, 0);
            mt.coeffRef(3, 3) = 1.0;
            
            // 残差計算
            KSVectorSparsef v(4);
            for(int i=0,n=d.rows()/3; i<n; ++i)
            {
                v.coeffRef(0) = data.coeff(0, 3*i);
                v.coeffRef(1) = data.coeff(0, 3*i+1);
                v.coeffRef(2) = data.coeff(0, 3*i+2);
                v.coeffRef(3) = 1.0f;
                
                // r = a - v * mt
                KSVectorSparsef vmt = v.transpose() * mt;
                d.coeffRef(3*i,0)    = data.coeff(1, 3*i) - vmt.coeff(0);
                d.coeffRef(3*i+1,0)  = data.coeff(1, 3*i+1) - vmt.coeff(1);
                d.coeffRef(3*i+2,0)  = data.coeff(1, 3*i+2) - vmt.coeff(2);
            }
            return d;
        };
        
        // 残差のヤコビアン
        KSFunctionSparse jacobian     = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef d(data.cols(), x.rows());
            
            float a = x.coeff(0, 0);
            float b = x.coeff(1, 0);
            float c = x.coeff(2, 0);
            
            for(int i=0,n=d.rows()/3; i<n; ++i)
            {
                // 極めてΘが小さい場合は
                // cosΘ = 1, sinΘ = Θ
                // さらに、sin * sin = 0と近似できる。
                // これを踏まえて行列を変換すると、以下のようになる
                // (X軸角がa, Y軸角がb, Z軸角がc, 平行移動が(tx,ty,tz))
                //      | 1   -c  b   0 |
                // RT = | c   1   -a  0 |
                //      | -b  a   1   0 |
                //      | tx  ty  tz  1 |
                // つまり、v(x, y, z, 1)を変換すると、
                //          | x + cy -bz + tx   |t
                // v * RT = | -cx + y + az + ty |
                //          | bx - ay + z + tz  |
                //          | 1                 |
                // になる。
                // ということで、ヤコビアンは、、、
                
                float vx = data.coeff(0, 3*i+0);
                float vy = data.coeff(0, 3*i+1);
                float vz = data.coeff(0, 3*i+2);
                
                //6個入れる
                d.coeffRef(3*i, 0) = 0.0;
                d.coeffRef(3*i, 1) = -(-vz);
                d.coeffRef(3*i, 2) = -(vy);
                d.coeffRef(3*i, 3) = -1.0;
                d.coeffRef(3*i, 4) = 0.0;
                d.coeffRef(3*i, 5) = 0.0;
                
                //6個入れる
                d.coeffRef(3*i+1, 0) = -(vz);
                d.coeffRef(3*i+1, 1) = 0.0;
                d.coeffRef(3*i+1, 2) = -(-vx);
                d.coeffRef(3*i+1, 3) = 0.0;
                d.coeffRef(3*i+1, 4) = -1.0;
                d.coeffRef(3*i+1, 5) = 0.0;
                
                //6個入れる
                d.coeffRef(3*i+2, 0) = -(-vy);
                d.coeffRef(3*i+2, 1) = -(vx);
                d.coeffRef(3*i+2, 2) = 0.0;
                d.coeffRef(3*i+2, 3) = 0.0;
                d.coeffRef(3*i+2, 4) = 0.0;
                d.coeffRef(3*i+2, 5) = -1.0;
                
                // 以下は各軸の検証
                    /*
                     // X軸回転
                     //6個入れる
                     d.coeffRef(3*i, 0) = 0.0;
                     d.coeffRef(3*i, 1) = 0.0;
                     d.coeffRef(3*i, 2) = 0.0;
                     d.coeffRef(3*i, 3) = -1.0;
                     d.coeffRef(3*i, 4) = 0.0;
                     d.coeffRef(3*i, 5) = 0.0;
                     
                     //6個入れる
                     d.coeffRef(3*i+1, 0) = -(vx);
                     d.coeffRef(3*i+1, 1) = 0.0;
                     d.coeffRef(3*i+1, 2) = 0.0;
                     d.coeffRef(3*i+1, 3) = 0.0;
                     d.coeffRef(3*i+1, 4) = -1.0;
                     d.coeffRef(3*i+1, 5) = 0.0;
                     
                     //6個入れる
                     d.coeffRef(3*i+2, 0) = -(-vy);
                     d.coeffRef(3*i+2, 1) = 0.0;
                     d.coeffRef(3*i+2, 2) = 0.0;
                     d.coeffRef(3*i+2, 3) = 0.0;
                     d.coeffRef(3*i+2, 4) = 0.0;
                     d.coeffRef(3*i+2, 5) = -1.0;
                    */
                    
                    /*
                     // Y軸回転
                    //6個入れる
                    d.coeffRef(3*i, 0) = 0.0;
                    d.coeffRef(3*i, 1) = -(-vz);
                    d.coeffRef(3*i, 2) = 0.0;
                    d.coeffRef(3*i, 3) = -1.0;
                    d.coeffRef(3*i, 4) = 0.0;
                    d.coeffRef(3*i, 5) = 0.0;
                    
                    //6個入れる
                    d.coeffRef(3*i+1, 0) = 0.0;
                    d.coeffRef(3*i+1, 1) = 0.0;
                    d.coeffRef(3*i+1, 2) = 0.0;
                    d.coeffRef(3*i+1, 3) = 0.0;
                    d.coeffRef(3*i+1, 4) = -1.0;
                    d.coeffRef(3*i+1, 5) = 0.0;
                    
                    //6個入れる
                    d.coeffRef(3*i+2, 0) = 0.0;
                    d.coeffRef(3*i+2, 1) = -(vx);
                    d.coeffRef(3*i+2, 2) = 0.0;
                    d.coeffRef(3*i+2, 3) = 0.0;
                    d.coeffRef(3*i+2, 4) = 0.0;
                    d.coeffRef(3*i+2, 5) = -1.0;
                    */
                    
                    /*
                     // Z軸回転
                     //6個入れる
                     d.coeffRef(3*i, 0) = 0.0;
                     d.coeffRef(3*i, 1) = 0.0;
                     d.coeffRef(3*i, 2) = -(vy);
                     d.coeffRef(3*i, 3) = -1.0;
                     d.coeffRef(3*i, 4) = 0.0;
                     d.coeffRef(3*i, 5) = 0.0;
                     
                     //6個入れる
                     d.coeffRef(3*i+1, 0) = 0.0;
                     d.coeffRef(3*i+1, 1) = 0.0;
                     d.coeffRef(3*i+1, 2) = -(-vx);
                     d.coeffRef(3*i+1, 3) = 0.0;
                     d.coeffRef(3*i+1, 4) = -1.0;
                     d.coeffRef(3*i+1, 5) = 0.0;
                     
                     //6個入れる
                     d.coeffRef(3*i+2, 0) = 0.0;
                     d.coeffRef(3*i+2, 1) = 0.0;
                     d.coeffRef(3*i+2, 2) = 0.0;
                     d.coeffRef(3*i+2, 3) = 0.0;
                     d.coeffRef(3*i+2, 4) = 0.0;
                     d.coeffRef(3*i+2, 5) = -1.0;
                     */
            }
            
            return d;
        };
        
        // 正解値マトリックスの初期値を設定
        KSMatrixSparsef param(paramNum,1);
        for (int i = 0; i < paramNum; ++i)
        {
            param.coeffRef(i, 0) = 0.0;
        }
        
        // オプティマイザの初期化
        optimizer.Initialize(residual, jacobian, param, data);
        
        std::vector<double> srsLog;
        
        // ニュートンステップは7回
        const int gaussStepNum = 7;
        TS_START("optimization exmple 4");
        for (int i = 0; i < gaussStepNum; ++i)
        {
            ofASSERT(optimizer.DoGaussNewtonStep(), "ガウス-ニュートン計算ステップに失敗しました。");
            srsLog.push_back(optimizer.GetSquaredResidualsSum());
        }
        TS_STOP("optimization exmple 4");
        
        //各ステップでの残差平方和
        for(int i = 0; i < gaussStepNum; ++i)
        {
            ofLog(OF_LOG_NOTICE, "SRS: (%dth step)%lf", i, srsLog[i]);
        }
        
        // 最適化結果パラメータを照会
        for (int i =0; i < paramNum; ++i)
        {
            ofLog(OF_LOG_NOTICE, "%dth param: [opt]%lf, [ans]%lf", i, optimizer.GetParamMat().coeff(i, 0), anser[i]);
            ofASSERT(fabs(optimizer.GetParamMat().coeff(i, 0) - anser[i]) < 0.01, "パラメータ推定結果が異なります。");
        }

    }
    
    // 例題No.5
    {
        // カメラパラメータを解いてみる
        // カメラは常に原点にあると考えると、ワールド->ビュー変換については考慮しなくていい。
        // よって透視投影だけ考える。
        // スクリーンの中心をカメラ空間のZ軸が通るとし、スクリーンがx軸上でlで交差し、y軸上でtで交差するとすると、
        //     n/l, 0,    0,            0
        // M = 0,   n/t,  0,            0
        //     0,   0,    -(f+n)/(f-n), -1
        //     0,   0,    -2nf/(f-n),   0
        // ここで、nはニアクリップ、fはファークリップである。
        // ニアクリップとファークリップは固定値で良いため、ここではn=1.0,f=1000.0とする。
        
        // 数値検証実験
        {
            float x = -10.0f;
            float y = 20.0f;
            float z = -30.0f;
            
            float l = 300.0f;
            float t = 200.0f;
            
            float n = 1.0f;
            float f = 1000.0f;
            float fov = ofRadToDeg(2.0f * atanf(t/n));
            float aspect = l / t;
            ofCamera cam;
            cam.setNearClip(n);
            cam.setFarClip(f);
            cam.setFov(fov);
            cam.setAspectRatio(aspect);
            ofMatrix4x4 projMat = cam.getProjectionMatrix();
            ofVec4f v = {x, y, z, 1.0f};
            ofVec4f a = v * projMat;
            
            KSMatrixSparsed mat(4,4);
            mat.coeffRef(0, 0) = n/l;
            mat.coeffRef(1, 1) = n/t;
            mat.coeffRef(2, 2) = -(f+n)/(f-n);
            mat.coeffRef(2, 3) = -1.0f;
            mat.coeffRef(3, 2) = -2.0f * n * f /(f -n );
            
            KSVectorSparsed vs(4);
            vs.coeffRef(0) = x;
            vs.coeffRef(1) = y;
            vs.coeffRef(2) = z;
            vs.coeffRef(3) = 1.0f;
            
            KSVectorSparsed as = vs.transpose() * mat;
            
            char buff[256];
            for (int i=0; i<4; ++i)
            {
                for (int j=0; j<4; ++j)
                {
                    sprintf(buff, "違うよ！(%d,%d), [eigen]:%lf, [of]:%lf",i,j,mat.coeff(i,j), projMat(i,j));
                    ofASSERT(fabs(mat.coeffRef(i, j) - projMat(i,j)) < 0.01, buff);
                }
            }
            
            for (int i=0; i<4; ++i)
            {
                sprintf(buff, "違うよ！%dth, [eigen]:%lf, [of]:%lf",i,a[i], as.coeff(i));
                ofASSERT(fabs(a[i] - as.coeff(i)) < 0.01, buff);
            }

        }
        
        // 正解値を適当に与える
        const int paramNum = 2;
        float anser[paramNum];
        anser[0] = 400.0f; // X軸交点
        anser[1] = 200.0f; // Y軸交点
        
        // プロジェクション行列作成
        float l = anser[0];
        float t = anser[1];
        float n = 1.0f;
        float f = 1000.0f;
        float fov = ofRadToDeg(2.0f * atanf(t/n));
        float aspect = l / t;
        ofCamera cam;
        cam.setNearClip(n);
        cam.setFarClip(f);
        cam.setFov(fov);
        cam.setAspectRatio(aspect);
        
        // 適当に入力データサンプルを作る
        int sampleVecNum = 1;
        KSMatrixSparsef data(2, 3 * sampleVecNum);
        for (int i = 0; i < sampleVecNum; ++i)
        {
            ofVec4f v;
            v.x = ofRandom(-100.0f, 100.0f);
            v.y = ofRandom(-100.0f, 100.0f);
            v.z = ofRandom(-100.0f, 100.0f);
            v.w = 1.0f;
            
            /*
            ofVec3f t = {0.0f, 0.0f, -20.0f};
            v += t;
            */
            
            ofVec4f a = v * cam.getProjectionMatrix();
             
            data.insert(0, 3*i+0) = v.x;
            data.insert(0, 3*i+1) = v.y;
            data.insert(0, 3*i+2) = v.z;
            
            data.insert(1, 3*i+0) = a.x;
            data.insert(1, 3*i+1) = a.y;
            data.insert(1, 3*i+2) = a.z;
        }
        
        // オプティマイザの宣言
        KSSparseOptimizer  optimizer;
        
        // ソルバを前処理付き共役勾配法(PGC)に変更
        optimizer.SwitchNormalEquationSolver(NESolverType::PCG);
        
        // PGCの試行回数のセット
        optimizer.SetMaxIterations(4);
        
        // 残差関数
        KSFunctionSparse  residual    = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef d(data.cols(), 1);
            
            float l = x.coeff(0, 0);
            float t = x.coeff(1, 0);
            float n = 1.0f;
            float f = 1000.0f;
            
            KSMatrixSparsef mt(4,4);
            
            mt.coeffRef(0, 0) = n/l;
            mt.coeffRef(1, 1) = n/t;
            mt.coeffRef(2, 2) = -(f+n)/(f-n);
            mt.coeffRef(2, 3) = -1.0f;
            mt.coeffRef(3, 2) = -2.0f * n * f /(f -n );
            
            // 残差計算
            KSVectorSparsef v(4);
            for(int i=0,n=d.rows()/3; i<n; ++i)
            {
                v.coeffRef(0) = data.coeff(0, 3*i);
                v.coeffRef(1) = data.coeff(0, 3*i+1);
                v.coeffRef(2) = data.coeff(0, 3*i+2);
                v.coeffRef(3) = 1.0f;
                
                // r = a - v * mt
                KSVectorSparsef vmt = v.transpose() * mt;
                d.coeffRef(3*i,0)    = data.coeff(1, 3*i) - vmt.coeff(0);
                d.coeffRef(3*i+1,0)  = data.coeff(1, 3*i+1) - vmt.coeff(1);
                d.coeffRef(3*i+2,0)  = data.coeff(1, 3*i+2) - vmt.coeff(2);
            }
            return d;
        };
        
        // 残差のヤコビアン
        KSFunctionSparse jacobian     = [&optimizer](const KSMatrixSparsef &x)->KSMatrixSparsef
        {
            const KSMatrixSparsef& data = optimizer.GetDataMat();
            KSMatrixSparsef d(data.cols(), x.rows());
            
            float l = x.coeff(0, 0);
            float t = x.coeff(1, 0);
            float n = 1.0f;
            float f = 1000.0f;
            
            for(int i=0,n=d.rows()/3; i<n; ++i)
            {
                //     n/l, 0,    0,            0
                // M = 0,   n/t,  0,            0
                //     0,   0,    -(f+n)/(f-n), -1
                //     0,   0,    -2nf/(f-n),   0
                //
                
                float vx = data.coeff(0, 3*i+0);
                float vy = data.coeff(0, 3*i+1);
                float vz = data.coeff(0, 3*i+2);
                
                //2個入れる
                d.coeffRef(3*i, 0) = -(-n*vx/(l*l));
                d.coeffRef(3*i, 1) = 0.0;
                
                //2個入れる
                d.coeffRef(3*i+1, 0) = 0.0;
                d.coeffRef(3*i+1, 1) = -(-n*vy/(t*t));
                
                //2個入れる
                d.coeffRef(3*i+2, 0) = 0.0;
                d.coeffRef(3*i+2, 1) = 0.0;
            }
            
            return d;
        };
        
        // 正解値マトリックスの初期値を設定
        KSMatrixSparsef param(paramNum,1);
        for (int i = 0; i < paramNum; ++i)
        {
            param.coeffRef(i, 0) = 100.0;
        }
        
        // オプティマイザの初期化
        optimizer.Initialize(residual, jacobian, param, data);
        
        std::vector<double> srsLog;
        
        // ニュートンステップは7回
        const int gaussStepNum = 7;
        TS_START("optimization exmple 5");
        for (int i = 0; i < gaussStepNum; ++i)
        {
            ofASSERT(optimizer.DoGaussNewtonStep(), "ガウス-ニュートン計算ステップに失敗しました。");
            srsLog.push_back(optimizer.GetSquaredResidualsSum());
        }
        TS_STOP("optimization exmple 5");
        
        //各ステップでの残差平方和
        for(int i = 0; i < gaussStepNum; ++i)
        {
            ofLog(OF_LOG_NOTICE, "SRS: (%dth step)%lf", i, srsLog[i]);
        }
        
        // 最適化結果パラメータを照会
        for (int i =0; i < paramNum; ++i)
        {
            ofLog(OF_LOG_NOTICE, "%dth param: [opt]%lf, [ans]%lf", i, optimizer.GetParamMat().coeff(i, 0), anser[i]);
            ofASSERT(fabs(optimizer.GetParamMat().coeff(i, 0) - anser[i]) < 0.01, "パラメータ推定結果が異なります。");
        }
    }
    
    return true;
}