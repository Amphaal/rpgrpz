pipeline {
    agent none
    stages {
        stage('Configure && Build') {
            parallel {
                stage('MinGW') {
                    agent {
                        docker { image 'amphaal/rpgrpz-ci-windows' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genWindows -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-ci.cmake'
                        sh 'ninja -C _genWindows'
                        withCredentials([string(credentialsId: 'jenkins-bintray-api-key', variable: 'BINTRAY_API_KEY')]) {
                            sh 'curl -T _genWindows/installer.zip   -uamphaal:$BINTRAY_API_KEY -H "X-Bintray-Package: install-packages" -H "X-Bintray-Version: latest" -H "X-Bintray-Publish: 1" -H "X-Bintray-Override: 1" -H "X-Bintray-Explode: 1" https://api.bintray.com/content/amphaal/rpgrpz/'
                            sh 'curl -T _genWindows/repository.zip  -uamphaal:$BINTRAY_API_KEY -H "X-Bintray-Package: ifw"              -H "X-Bintray-Version: latest" -H "X-Bintray-Publish: 1" -H "X-Bintray-Override: 1" -H "X-Bintray-Explode: 1" https://api.bintray.com/content/amphaal/rpgrpz/ifw-win64/'
                        }
                    }
                }
                stage('Linux') {
                    agent {
                        docker { image 'amphaal/rpgrpz-linux-ci' }
                    }
                    steps {
                        sh 'cmake -GNinja -B_genLinux -H. -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-ci.cmake'
                        sh 'ninja -C _genLinux'
                    }
                }
            }
        }
    }
}