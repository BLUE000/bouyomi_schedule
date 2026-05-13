pipeline {
    agent any
    
    parameters {
        choice(
            name: 'BUILD_TYPE',
            choices: ['individual', 'periodic'],
            description: 'ビルドタイプ(通知先の切り替え)'
        )
    }
    
    environment {
        WEBHOOK_PERIODIC = credentials('discord-webhook-periodic')
        WEBHOOK_INDIVIDUAL = credentials('discord-webhook-individual')
        
        QT_DIR = 'C:\\Qt\\6.11.0\\mingw_64'
        CMAKE_BIN = 'C:\\Qt\\Tools\\CMake_64\\bin'
        NINJA_BIN = 'C:\\Qt\\Tools\\Ninja'
        MINGW_BIN = 'C:\\Qt\\Tools\\mingw1310_64\\bin'
    }
    
    stages {
        stage('Debug Info') {
            steps {
                echo "=== Build Information ==="
                echo "BUILD_TYPE: ${params.BUILD_TYPE}"
                echo "GIT_BRANCH: ${env.GIT_BRANCH}"
                echo "BUILD_NUMBER: ${env.BUILD_NUMBER}"
            }
        }
        
        stage('Clean') {
            steps {
                bat 'if exist build_jenkins rmdir /s /q build_jenkins'
            }
        }
        
        stage('CMake Configuration') {
            steps {
                bat '''
                    set PATH=%QT_DIR%\\bin;%CMAKE_BIN%;%NINJA_BIN%;%MINGW_BIN%;%PATH%
                    cmake -G Ninja -B build_jenkins -DCMAKE_BUILD_TYPE=Debug
                '''
            }
        }
        
        stage('Build') {
            steps {
                bat '''
                    set PATH=%QT_DIR%\\bin;%CMAKE_BIN%;%NINJA_BIN%;%MINGW_BIN%;%PATH%
                    cmake --build build_jenkins --target unit_tests
                '''
            }
        }
        
        stage('Test') {
            steps {
                bat '.\\build_jenkins\\unit_tests.exe --gtest_output=xml:unit_tests_report.xml'
            }
        }
        
        stage('Notify Success') {
            steps {
                script {
                    def webhook = (params.BUILD_TYPE == 'periodic') ? env.WEBHOOK_PERIODIC : env.WEBHOOK_INDIVIDUAL
                    def message = "✅ **ビルド成功 [${params.BUILD_TYPE}]**\\n" +
                                  "Job: #${env.BUILD_NUMBER}\\n" +
                                  "Branch: ${env.GIT_BRANCH}\\n" +
                                  "URL: ${env.BUILD_URL}"
                    
                    bat "curl -X POST ${webhook} -H \"Content-Type: application/json\" -d \"{\\\"content\\\": \\\"${message}\\\"}\""
                }
            }
        }
    }
    
    post {
        always {
            junit allowEmptyResults: true, testResults: 'unit_tests_report.xml' 
        }
        
        failure {
            script {
                def webhook = (params.BUILD_TYPE == 'periodic') ? env.WEBHOOK_PERIODIC : env.WEBHOOK_INDIVIDUAL
                def message = "❌ **ビルド失敗 [${params.BUILD_TYPE}]**\\n" +
                              "Job: #${env.BUILD_NUMBER}\\n" +
                              "Branch: ${env.GIT_BRANCH}\\n" +
                              "URL: ${env.BUILD_URL}"
                
                bat "curl -X POST ${webhook} -H \"Content-Type: application/json\" -d \"{\\\"content\\\": \\\"${message}\\\"}\""
            }
        }
    }
}
