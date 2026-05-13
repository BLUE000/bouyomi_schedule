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
                    cmake --build build_jenkins
                '''
            }
        }
        
        stage('Notify Success') {
            steps {
                script {
                    def webhook = (params.BUILD_TYPE == 'periodic') ? env.WEBHOOK_PERIODIC : env.WEBHOOK_INDIVIDUAL
                    
                    // PowerShell で UTF-8 対応の通知を送信
                    powershell """
                        \$webhook = '${webhook}'
                        \$body = @{
                            content = ":white_check_mark: **ビルド成功 [${params.BUILD_TYPE}]**`nJob: #${env.BUILD_NUMBER}`nBranch: ${env.GIT_BRANCH}`nURL: ${env.BUILD_URL}"
                        } | ConvertTo-Json
                        
                        [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12
                        Invoke-RestMethod -Uri \$webhook -Method Post -Body \$body -ContentType 'application/json; charset=utf-8'
                    """
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
                
                // PowerShell で UTF-8 対応の通知を送信
                powershell """
                    \$webhook = '${webhook}'
                    \$body = @{
                        content = ":x: **ビルド失敗 [${params.BUILD_TYPE}]**`nJob: #${env.BUILD_NUMBER}`nBranch: ${env.GIT_BRANCH}`nURL: ${env.BUILD_URL}"
                    } | ConvertTo-Json
                    
                    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12
                    Invoke-RestMethod -Uri \$webhook -Method Post -Body \$body -ContentType 'application/json; charset=utf-8'
                """
            }
        }
    }
}
