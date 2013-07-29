#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

#include "cinder/Utilities.h"
#include "cinder/gl/GlslProg.h"

#include "ParticleSystem.h"
#include "cinder/audio/Io.h"
#include "cinder/audio/FftProcessor.h"
#include "cinder/audio/PcmBuffer.h"
#include "cinder/audio/Output.h"
#include "cinder/Utilities.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class ReactiveApp : public AppBasic {
public:
	void setup();
    void mouseMove(MouseEvent event);
    void mouseDown(MouseEvent event);
    void keyDown(KeyEvent event);
    void update();
    void draw();
	
    ParticleSystem mParticleSystem;
    Vec2f   attrPosition;
    float   attrFactor, repulsionFactor;
	audio::TrackRef mAudio;

    bool    mRunning;
	gl::GlslProg	mMetaballsShader;
	int mNumParticles;
};

void ReactiveApp::setup()
{
    setWindowSize(640, 480);
	setFullScreen(true);
    mRunning = true;
    attrPosition = getWindowCenter();
    attrFactor = 0.5f;
    repulsionFactor = 5.5f;
	

	mNumParticles = 16;
    for( int i=0; i<mNumParticles; i++ ){
        float x = ci::randFloat( 0.0f, getWindowWidth() );
        float y = ci::randFloat( 0.0f, getWindowHeight() );
        float radius = ci::randFloat( 2.2f, 10.0f );
        float mass = radius;
        float drag = 0.9f;
        Particle *particle = new Particle( Vec2f( x, y ), radius, mass, drag );
        mParticleSystem.addParticle( particle );
    }
	mMetaballsShader = gl::GlslProg( loadAsset( "passThru_vert.glsl" ), loadAsset( "mb_frag.glsl" )  );

    fs::path audioPath = getOpenFilePath();
    if( audioPath.empty() == false ){
        mAudio = audio::Output::addTrack( audio::load( audioPath.string() ) );
    }
    if( mAudio ){
        mAudio->enablePcmBuffering( true );
        mAudio->setLooping( true );
        mAudio->play();
		hideCursor();
    }
}

void ReactiveApp::mouseMove(MouseEvent event)
{
    attrPosition.x = event.getPos().x;
    attrPosition.y = event.getPos().y;
}

void ReactiveApp::mouseDown(MouseEvent event)
{
    for( std::vector<Particle*>::iterator it = mParticleSystem.particles.begin(); it != mParticleSystem.particles.end(); ++it ) {
        Vec2f repulsionForce = (*it)->position - event.getPos();
        repulsionForce = repulsionForce.normalized() * math<float>::max(0.f, 100.f - repulsionForce.length());
        (*it)->forces += repulsionForce * repulsionFactor;
    }
}

void ReactiveApp::keyDown(KeyEvent event) {
    if(event.getChar() == ' ') {
        mRunning = !mRunning;
		if (mAudio && mAudio->isPlaying()) {
			mAudio->stop();
		} else if (mAudio) {
			mAudio->play();
		}
    }
}

void ReactiveApp::update() {
    
    if(!mRunning)
        return;

    for( std::vector<Particle*>::iterator it = mParticleSystem.particles.begin(); it != mParticleSystem.particles.end(); ++it ) {
        Vec2f attrForce = attrPosition - (*it)->position;
        attrForce *= attrFactor;
        (*it)->forces += attrForce;
    }
	
    Vec2f center = getWindowCenter();
    for( vector<Particle*>::iterator it = mParticleSystem.particles.begin(); it != mParticleSystem.particles.end(); ++it ){
        Particle *particle = *it;
        Vec2f force = ( center - particle->position ) * 0.1f;
        particle->forces += force;
    }
    
    std::shared_ptr<float> fft;
    if( mAudio ){
        audio::PcmBuffer32fRef pcmBuffer = mAudio->getPcmBuffer();
        if( pcmBuffer ){
            fft = audio::calculateFft( pcmBuffer->getChannelData( audio::CHANNEL_FRONT_LEFT ), mNumParticles );
        }
    }
    
    if( fft ){
        float *values = fft.get();
        for( int i=0; i<mParticleSystem.particles.size()-1; i++ ){
            for( int j=i+1; j<mParticleSystem.particles.size(); j++ ){
                Particle *particleA = mParticleSystem.particles[i];
                Particle *particleB = mParticleSystem.particles[j];
                Vec2f delta = particleA->position - particleB->position;
                float distanceSquared = delta.lengthSquared();
                particleA->forces += ( delta / distanceSquared ) * particleB->mass * values[j] * 50.5f;
                particleB->forces -= ( delta / distanceSquared ) * particleA->mass * values[i] * 50.5f;
            }
        }
    }
    mParticleSystem.update();
}

void ReactiveApp::draw()
{
    gl::enableAlphaBlending();
	gl::clear( Color::black() );
    gl::setMatricesWindow(getWindowSize());
	gl::setViewport(getWindowBounds());
    
    int particleNum = mParticleSystem.particles.size();
    
    mMetaballsShader.bind();
    mMetaballsShader.uniform( "size", Vec2f(getWindowSize()) );
    mMetaballsShader.uniform( "num", particleNum );
    
    for (int i = 0; i < particleNum; i++) {
		mMetaballsShader.uniform( "positions[" + toString(i) + "]", mParticleSystem.particles[i]->position );
		mMetaballsShader.uniform( "radius[" + toString(i) + "]", mParticleSystem.particles[i]->radius );
	}
    
    gl::color(Color::white());
    gl::drawSolidRect( getWindowBounds() );
	mMetaballsShader.unbind();    
}

CINDER_APP_BASIC( ReactiveApp, RendererGl )