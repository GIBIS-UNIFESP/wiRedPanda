
$('.banner').hide();
$('.features').hide();
$('.banner-title').hide();
$('.banner-subtitle').hide();
/*====================================
=            DOM IS READY            =
====================================*/

$(function() {
    $('.banner').animate({
      // width: [ "toggle", "swing" ],
      opacity: "toggle"
    },1000);

    $('.banner-title').animate({
      opacity: "toggle"
    },1500);

    $('.banner-subtitle').animate({
      opacity: "toggle"
    },2000);


    $('.features').animate({opacity:'toggle'},1000);

    $('.pagination .active a').click(function() {
        return false;
    });
});


/*========================================
=            WINDOW IS LOADED            =
========================================*/
$(window).load(function() {

});


/*=========================================
=            WINDOW IS RESIZED            =
=========================================*/
$(window).resize(function() {

});


/*==========================================
=            WINDOW IS SCROLLED            =
==========================================*/
$(window).scroll(function() {
});
