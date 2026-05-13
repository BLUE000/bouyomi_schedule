pipeline {
    agent any
    
    environment {
        WEBHOOK_PERIODIC = credentials('discord-webhook-periodic')
        WEBHOOK_INDIVIDUAL = credentials('discord-webhook-individual')
    }
    
    stages {
        stage('Build') {
            steps {
                echo "ビルドを開始します..."
                echo "ブランチ: ${env.GIT_BRANCH}"  // 実際のブランチ名を表示
                bat '''
                    mkdir build
                    cd build
                    cmake -G "MinGW Makefiles" ..
                    cmake --build .
                '''
            }
        }
        
        stage('Notify Discord') {
            steps {
                script {
                    def webhook = (params.BUILD_TYPE == 'periodic') ? env.WEBHOOK_PERIODIC : env.WEBHOOK_INDIVIDUAL
                    def branchName = env.GIT_BRANCH ?: 'unknown'
                    bat """
                        curl -X POST ${webhook} -H "Content-Type: application/json" -d "{\\"content\\": \\"✅ ビルド成功: ${branchName} (${params.BUILD_TYPE})\\"}"
                    """
                }
            }
        }
    }
    
    post {
        failure {
            script {
                def webhook = (params.BUILD_TYPE == 'periodic') ? env.WEBHOOK_PERIODIC : env.WEBHOOK_INDIVIDUAL
                def branchName = env.GIT_BRANCH ?: 'unknown'
                bat """
                    curl -X POST ${webhook} -H "Content-Type: application/json" -d "{\\"content\\": \\"❌ ビルド失敗: ${branchName} (${params.BUILD_TYPE})\\"}"
                """
            }
        }
    }
}
