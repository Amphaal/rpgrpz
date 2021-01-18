pipeline {
    agent none
    stages {
        // stage('Configure && Build') {
            // parallel {
                stage('MinGW') {
                    agent {
                        docker { image 'amphaal/rpgrpz-ci-windows' }
                    }
                    environment {
                        SENTRY_URL = 'https://sentry.io/'
                        SENTRY_ORG = 'lvwl'
                        SENTRY_PROJECT = 'rpgrpz'
                    }
                    steps {
                        sh 'cmake -GNinja -B_genWindows -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake'
                        sh 'cmake --build ./_genWindows --target zipForDeploy'
                        withCredentials([string(credentialsId: 'jenkins-sentry-auth-token', variable: 'SENTRY_AUTH_TOKEN')]) {
                            sh 'sentry-cli --auth-token $SENTRY_AUTH_TOKEN releases new -p rpgrpz $(cat _genWindows/version)'
                            sh 'sentry-cli --auth-token $SENTRY_AUTH_TOKEN releases set-commits --auto $(cat _genWindows/version)'
                            sh 'sentry-cli --auth-token $SENTRY_AUTH_TOKEN upload-dif --wait _genWindows/out/RPGRPZ.exe _genWindows/out/RPGRPZ.pdb'
                        }
                        withCredentials([string(credentialsId: 'jenkins-bintray-api-key', variable: 'BINTRAY_API_KEY')]) {
                            sh 'curl -T _genWindows/installer.zip   -uamphaal:$BINTRAY_API_KEY -H "X-Bintray-Package: install-packages" -H "X-Bintray-Version: latest" -H "X-Bintray-Publish: 1" -H "X-Bintray-Override: 1" -H "X-Bintray-Explode: 1" https://api.bintray.com/content/amphaal/rpgrpz/'
                            sh 'curl -T _genWindows/repository.zip  -uamphaal:$BINTRAY_API_KEY -H "X-Bintray-Package: ifw"              -H "X-Bintray-Version: latest" -H "X-Bintray-Publish: 1" -H "X-Bintray-Override: 1" -H "X-Bintray-Explode: 1" https://api.bintray.com/content/amphaal/rpgrpz/ifw-win64/'
                        }
                    }
                }
                // stage('Linux') {
                //     agent {
                //         docker { image 'amphaal/rpgrpz-ci-linux' }
                //     }
                //     steps {
                //         sh 'cmake -GNinja -B_genLinux -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-ci.cmake'
                //         sh 'ninja -C _genLinux'
                //     }
                // }
            // }
        // }
    }
}
